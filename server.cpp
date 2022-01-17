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
#define PORT 5000

// server messages
#define FILE_DOES_NOT_EXIST "server file doesn't exists"
#define FILE_EMPTY "server file is empty"
#define FILE_INDX_OUT_RANGE "query line is out of range of server file"
#define FILE_WRITE_FAILED "failed to insert to server file"
#define FILE_WRITE_SUCCESS "successfully written to file"
#define WRONG_COMMAND "command not recognized"

using namespace std;

int Lines;

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
			count++;
		}
	}
	fclose(fp);
	return count;
}

string putLine(string s, int k)
{
	make_copy();
	FILE* fp1 = fopen("server_file_temp.txt", "r");
	FILE* fp2 = fopen("server_file.txt", "w");
	if(Lines == -1)
		Lines = countLines();
	if(k < 0)
	{
		k = Lines + k;
	}
	ssize_t read;
	char* line;
	size_t len = 0;
	int count = 0;
	bool flg = false;

	if(fp2 != NULL && fp1 != NULL)
	{	
		if(s[s.length()-1] == '\n')
		{
			s = s.substr(0, s.length()-1);
		}
		while(read = getline(&line, &len, fp1) != -1)
		{
			if(count == k)
			{
				s = to_string(k) + "\t" + s + "\n";
				fputs(s.c_str(), fp2);
				flg = true;
			}
			count++;
			fputs(line, fp2);
		}
		if(flg)
		{
			fclose(fp1);
			fclose(fp2);
			return FILE_WRITE_SUCCESS;
		}
		else
		{
			fclose(fp1);
			fclose(fp2);
			return FILE_WRITE_FAILED;
		}
	}
	else
	{
		fclose(fp1);
		fclose(fp2);
		return FILE_DOES_NOT_EXIST;
	}
}

string putLine(string s)
{
	make_copy();
	FILE* fp1 = fopen("server_file_temp.txt", "r");
	FILE* fp2 = fopen("server_file.txt", "w");
	
	ssize_t read;
	char* line;
	size_t len = 0;
	int count = 0;

	if(fp2 != NULL && fp1 != NULL)
	{	
		if(s[s.length()-1] == '\n')
		{
			s = s.substr(0, s.length()-1);
		}
		while(read = getline(&line, &len, fp1) != -1)
		{
			count++;
			fputs(line, fp2);
		}
		s = "\n" + to_string(count+1) + "\t" + s;
		fputs(s.c_str(), fp2);
		fclose(fp1);
		fclose(fp2);
		return FILE_WRITE_SUCCESS;
	}
	else
	{
		fclose(fp1);
		fclose(fp2);
		return FILE_DOES_NOT_EXIST;
	}
}

string readLine(int k)
{
	FILE* fp = fopen("server_file.txt", "r");
	ssize_t read;
	char* line;
	size_t len = 0;
	int count = 0;
	
	if(k < 0)
	{
		if(Lines == -1)
		{
			Lines = countLines();
		}
		k = Lines + k;
	}
	
	if(fp == NULL)
	{
		// perror("error reading file");
		return FILE_DOES_NOT_EXIST;
	}
	else
	{
		while((read = getline(&line, &len, fp)) != -1)
		{
			if(count == k)
			{
				fclose(fp);
				return string(line);
			}
			count++;
		}
	}
	fclose(fp);
	return FILE_EMPTY;
}

int main(int argc, char const *argv[])
{
	int sock_fd, new_sock, valread;
	struct sockaddr_in address;
	int opt = 1, i, j;
	int addrlen = sizeof(address);
	char buffer[1024];

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
}