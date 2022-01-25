#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <regex>
#define PORT 5000

#define WRONG_COMMAND "wrong command"

using namespace std;

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
	int sock_fd, n;
	char buffer[1024];
	string s;
	regex reg("(\\/\\W*(merge|sort|similarity)\\W*\\s(.*\\.txt\\s?)+\\s?[D|N|P]?)");
	smatch m;
	// smatch match;
	struct hostent* server;
	server = gethostbyname("localhost");

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	serv_addr.sin_port = htons(PORT);

	if (connect(sock_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
		perror("ERROR connecting");
	}

	getline(cin, s);

	while(s.compare("/exit") != 0)
	{
		if(regex_match(s, m, reg))
		{
			// cout << "boooyay\n" << m[0];
			// break;
			n = write(sock_fd, s.c_str(), s.length());
			bzero(buffer, 1024);
			if(s.rfind("/sort", 0) == 0)
			{
				cout << "sorting\n";
				int count;
				string* str = split(s, ' ', count);
				if(count != 3)
				{
					cout << WRONG_COMMAND << endl;
					continue;
				}
				ifstream fi(str[1]);
				char by = str[2][0];
				while(!fi.eof())
				{
					getline(fi, s);
					write(sock_fd, s.c_str(), s.length());
				}
				s = "eof";
				fi.close();
				write(sock_fd, s.c_str(), s.length());
				bzero(buffer, 1024);
				n = read(sock_fd,buffer,1024);
				s = buffer;
				ofstream fo(s);
				bzero(buffer, 1024);
				n = read(sock_fd,buffer,1024);
				s = buffer;
				while(s.compare("eof") != 0)
				{
					fo << s << endl;
					bzero(buffer, 1024);
					n = read(sock_fd,buffer,1024);
					s = buffer;
				}
				fo.close();
			}
			if(s.rfind("/merge", 0) == 0)
			{
				int count;
				string* str = split(s, ' ', count);
				if(count != 4)
				{
					cout << WRONG_COMMAND << endl;
					continue;
				}
				char by = str[3][0];
				ifstream fi1(str[1]), fi2(str[2]);
				while(!fi1.eof())
				{
					getline(fi1, s);
					write(sock_fd, s.c_str(), s.length());
				}
				s = "eof";
				write(sock_fd, s.c_str(), s.length());
				while(!fi2.eof())
				{
					getline(fi2, s);
					write(sock_fd, s.c_str(), s.length());
				}
				s = "eof";
				write(sock_fd, s.c_str(), s.length());
				fi1.close();fi2.close();
				bzero(buffer, 1024);
				n = read(sock_fd,buffer,1024);
				s = buffer;
				ofstream fo(s);
				bzero(buffer, 1024);
				n = read(sock_fd,buffer,1024);
				s = buffer;
				while(s.compare("eof") != 0)
				{
					fo << s << endl;
					bzero(buffer, 1024);
					n = read(sock_fd,buffer,1024);
					s = buffer;
				}
				fo.close();
			}
			if(s.rfind("/similarity", 0) == 0)
			{
				int count;
				string* str = split(s, ' ', count);
				if(count != 3)
				{
					cout << WRONG_COMMAND << endl;
					continue;
				}
				ifstream fi1(str[1]), fi2(str[2]);
				while(!fi1.eof())
				{
					getline(fi1, s);
					write(sock_fd, s.c_str(), s.length());
				}
				s = "eof";
				write(sock_fd, s.c_str(), s.length());
				while(!fi2.eof())
				{
					getline(fi2, s);
					write(sock_fd, s.c_str(), s.length());
				}
				s = "eof";
				write(sock_fd, s.c_str(), s.length());
				fi1.close();fi2.close();
				bzero(buffer, 1024);
				n = read(sock_fd,buffer,1024);
				s = buffer;
				while(s.compare("eof"))
				{
					cout << s << endl;
					bzero(buffer, 1024);
					n = read(sock_fd,buffer,1024);
					s = buffer;
				}
			}
			getline(cin, s);
			// cout << s << "\n";
		}
		else
		{
			cout << "Wrong command\n";
			getline(cin, s);
		}
	}
	close(sock_fd);
}