#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#define PORT 5000

// server messages
#define FILE_DOES_NOT_EXIST "server file doesn't exists"
#define FILE_EMPTY "server file is empty"
#define FILE_INDX_OUT_RANGE "query line is out of range of server file"
#define FILE_WRITE_FAILED "failed to insert to server file"
#define FILE_WRITE_SUCCESS "successfully written to file"
#define WRONG_COMMAND "command not recognized"

using namespace std;

vector<ssize_t> offsets;
int fd;

int Lines = -1;

void make_copy()
{
	int read_fd;
	int write_fd;
	struct stat stat_buf;
	off_t offset = 0;
	read_fd = open ("server_file.txt", O_RDONLY);
	fstat (read_fd, &stat_buf);
	write_fd = open ("server_file_temp.txt", O_WRONLY | O_CREAT, stat_buf.st_mode);
	sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
	close(read_fd);
	close(write_fd);
}

int countLines()
{
	FILE* fp = fopen("server_file.txt", "r");
	ssize_t read;
	char* line;
	size_t len = 0;
	int count = 0;
	
	if(fp == NULL)
	{
		return -1;
	}
	else
	{
		while((read = getline(&line, &len, fp)) != -1)
		{
			offsets.push_back(read);
			count++;
		}
	}
	for(int i = 1; i < offsets.size(); i++)
	{
		offsets[i] = offsets[i] + offsets[i-1];
	}
	fclose(fp);
	return count;
}

string putLine(string s, int k)
{
	if(Lines == -1)
		Lines = countLines();
	if(k < 0)
	{
		k = Lines + k;
	}
	if(k >= Lines)
		return FILE_INDX_OUT_RANGE;
	else
	{
		lseek(fd, 0, SEEK_SET);
		string fileContent;
		char buffer[1024];
		ssize_t bytesRead;
		while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
		{
			fileContent.append(buffer, bytesRead);
		}
		vector<string> lines;
		stringstream ss(fileContent);
		string line;
		while (getline(ss, line))
		{
			if(k == lines.size())
			{
				lines.push_back(s);
				lines.push_back(line);
			}
			else {
				lines.push_back(line);
			}
		}
		lseek(fd, 0, SEEK_SET);
		ftruncate(fd, 0); // Clear the file content
		for (int i = 0; i < lines.size(); i++)
		{
			string lineToWrite;
			if(i != lines.size() - 1)
				lineToWrite = lines[i] + "\n";
			else 
				lineToWrite = lines[i];
			write(fd, lineToWrite.c_str(), lineToWrite.length());
		}
		return FILE_WRITE_SUCCESS;
	}
}

string putLine(string s)
{
	lseek(fd, 0, SEEK_END);
	write(fd, "\r\n", 1);
	write(fd, s.c_str(), s.length());
	return FILE_WRITE_SUCCESS;
}

string readLine(int k)
{
	if(k < 0) {
		if(Lines == -1)
		{
			Lines = countLines();
		}
		k = Lines + k;
	}
	if(Lines == 0)
		return FILE_EMPTY;
	if(k >= Lines)
		return FILE_INDX_OUT_RANGE;
	else
	{
		lseek(fd, offsets[k-1], SEEK_SET);
		char c;
		string s = "";
		while(read(fd, &c, 1) > 0 && c != '\n')
		{
			s += c;
		}
		s += '\0';
		return s;
	}
}

int main(int argc, char const *argv[])
{
	int sock_fd, new_sock, valread;
	struct sockaddr_in address;
	int opt = 1, i, j;
	int addrlen = sizeof(address);
	char buffer[1024];

	fd = open("server_file.txt", O_RDWR);
	if(fd == -1)
	{
		perror("error opening file");
		exit(EXIT_FAILURE);
	}

	bzero((char *) &address, sizeof(address));

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	// Forcefully attaching socket to the port 8080
	if (bind(sock_fd, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(sock_fd, 5) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in cli_addr;
	socklen_t clilen;
	clilen = sizeof(cli_addr);

	if ((new_sock = accept(sock_fd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen))<0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}

	valread = read( new_sock , buffer, 1024);
	string s(buffer);

	while(valread > 0 && s.compare("exit") != 0) {
		// cout << "clent: " << s << "\n";
		if(s.compare("NLINEX") == 0)
		{
			Lines = countLines();
			if(Lines == -1)
			{
				s = FILE_DOES_NOT_EXIST;
			}
			else
				s = to_string(Lines);
		}
		else if(s.rfind("READX", 0) == 0)
		{
			s = readLine(stoi(s.substr(5)));
			if(s[s.length() - 1] == '\n')
				s = s.substr(0, s.length() - 1);
		}
		else if(s.rfind("INSERTX", 0) == 0)
		{
			s = s.substr(8);
			if(s[0] >= '0' && s[0] <= '9')
			{
				i = s.find(' ');
				j = stoi(s.substr(0, i));
				s = putLine(s.substr(i + 1), j);
			}
			else
			{
				s = putLine(s);
			}
		}
		else
			s = WRONG_COMMAND;
		// else if(s.rfind("READX", 0) == 0)
		bzero(buffer, 1024);
		// getline(cin, s);
		send(new_sock, s.c_str(), s.length(), 0);
		valread = read( new_sock , buffer, 1024);
		s = buffer;
	}
	close(sock_fd);
	close(fd);
}