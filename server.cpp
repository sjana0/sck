#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#define PORT 5000

using namespace std;


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

int main(int argc, char const *argv[])
{
	int sock_fd, new_sock, valread;
	struct sockaddr_in address;
	int opt = 1;
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

	// if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	// {
	// 	perror("setsockopt");
	// 	exit(EXIT_FAILURE);
	// }

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
		cout << "clent: " << s << "\n";
		if(s.rfind("NLINEX") == 0)
		{
			s = to_string(countLines());
		}
		else
		{
			getline(cin, s);
		}
		bzero(buffer, 1024);
		send(new_sock, s.c_str(), s.length(), 0);
		valread = read( new_sock , buffer, 1024);
		s = buffer;
	}
	close(sock_fd);
}