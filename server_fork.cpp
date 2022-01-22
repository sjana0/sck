#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/shm.h>
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
#define SERVER_BUSY "server is busy"
#define MAX_CHILD 2

// semaphore operations

// struct sembuf pop, vop;

// #define P(s) semop(s, &pop, 1);
// #define V(s) semop(s, &vop, 1);


using namespace std;

int Lines = -1;

int shmid = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT);

string* split(string s,char c, int& count) {
	string static strar[1000];
	count = 0;
	int i = 0, j = 0;
	for(unsigned int p = 0; p < s.length(); p++) {
		if(s[p] == c) {
			strar[count++] = s.substr(i,j-i);
			i = j+1;
		}
		j++;
	}
	strar[count] = s.substr(i,j-i);
    count++;
	return strar;
}


int main()
{
	// initialize shared memory
	int *a;
	a = (int*)shmat(shmid, 0, 0);
	a[0] = 2;

	pid_t p;


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

	// Forcefully attaching socket to the port 5000
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
	while(1)
	{
		if(a[0] != 0)
		{
			if ((new_sock = accept(sock_fd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			p = fork();
		}
		else
		{
			cout << "doing it\n";
			close(new_sock);
			while(a[0] == 0);
		}
		if(p)
		{
			// close(new_sock);
			if(a[0] != 0)
			{
				a[0]--;
				cout << "client new "<<a[0] << "\n";
			}
			else
			{
			}
			
		}
		else
		{
			// child process: process command from client
			close(sock_fd);

			valread = read( new_sock , buffer, 1024);
			string s(buffer);

			while(valread > 0 && s.compare("exit") != 0) {
				cout << s << "\n";
				bzero(buffer, 1024);
				getline(cin, s);
				send(new_sock, s.c_str(), s.length(), 0);
				valread = read( new_sock , buffer, 1024);
				s = buffer;
			}
			close(new_sock);
			a[0]++;
			cout << "making shm leaving " << a[0] << "\n";
			break;
			// child process
		}
	}

}