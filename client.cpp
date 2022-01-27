#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <regex>
#define PORT 5000

#define WRONG_COMMAND "wrong command"
#define FILE_NOT_EXISTS "file doesn't exists"
#define WRONG_NUMBER_FILE "wrong number of file supplied"
#define WRONG_FIELD "wrong field axis"
#define SUCCESSFUL_CONNECTION "successful connection"
#define SERVER_BUSY "unsuccessful connection server is busy(MAX 5 client reached)"

using namespace std;

/***
only sends the content of the file
so has filename
and sock
***/

void send_file(string filename, int sock)
{
	char buffer[1024];
	string s;

	// wirte filename to server
	ifstream fi(filename);
	bzero(buffer, 1024);
	strcpy(buffer, filename.c_str());
	write(sock, buffer, filename.length());
	buffer[0] = '\0';
	bzero(buffer, 1024);
	
	while(!fi.eof())
	{
		// it reads first ack for filename from server
		buffer[0] = '\0';
		bzero(buffer, 1024);
		read(sock, buffer, 1024);
		
		s = buffer;
		buffer[0] = '\0';
		bzero(buffer, 1024);
		
		if(s.compare("acknowledged") == 0)
		{
			// cout << "inside send function3\n" << s << "\n";
			// reads first line from file and sends
			getline(fi, s);
			strcpy(buffer, s.c_str());

			write(sock, buffer, s.length());

			buffer[0] = '\0';
			bzero(buffer, 1024);
		}
	}
	read(sock, buffer, 1024);
	s = buffer;
	buffer[0] = '\0';
	bzero(buffer, 1024);
	if(s.compare("acknowledged") == 0)
	{
		s = "eof";
		strcpy(buffer, s.c_str());
		write(sock, buffer, s.length());
		buffer[0] = '\0';
		bzero(buffer, 1024);
	}
}

/***
filename send from server and sock
***/

string rcv_file(int sock)
{
	char buffer[1024];
	string s;
	
	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	string filename = buffer;
	buffer[0] = '\0';
	bzero(buffer, 1024);
	
	ofstream fo(filename);
	
	s = "acknowledged";
	strcpy(buffer, s.c_str());
	write(sock, buffer, s.length());
	
	buffer[0] = '\0';
	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	s = buffer;
	buffer[0] = '\0';
	bzero(buffer, 1024);
	
	string s1;
	while(s.compare("eof") != 0)
	{
		s1 = s;

		s = "acknowledged";
		strcpy(buffer, s.c_str());
		write(sock, buffer, s.length());
		
		buffer[0] = '\0';
		bzero(buffer, 1024);
		read(sock, buffer, 1024);
		s = buffer;
		buffer[0] = '\0';
		bzero(buffer, 1024);
		if(s.compare("eof") == 0)
			fo << s1;
		else
			fo << s1 << endl;
	}
	buffer[0] = '\0';
	bzero(buffer, 1024);
	fo.close();
	return filename;
}

bool command_check(string s, string& err)
{
	string s1;
	if(s.rfind("/sort") == 0 || s.rfind("/merge") == 0 || s.rfind("/similarity") == 0)
	{
		if((s.rfind("/sort", 0) == 0 || s.rfind("/merge", 0) == 0) && (s[s.length()-1] == 'D' || s[s.length()-1] == 'N' || s[s.length()-1] == 'P'))
		{
			int p = s.find(" ") + 1;
			int q = s.rfind(" ");
			s1 = s.substr(p, q-p);
			int j = 0;
			for(int i = 0; i < s1.length(); i++)
			{
				if(s1[i] == ' ')
				{
					j++;
				}
			}
			if(s.rfind("/sort", 0) == 0 && j == 0)
			{
				err = "";
				return true;
			}
			else if(s.rfind("/merge", 0) == 0 && j == 2)
			{
				err = "";
				return true;
			}
			else
			{
				err = WRONG_NUMBER_FILE;
				return false;
			}

		}
		else if(s.rfind("/similarity") == 0)
		{
			int p = s.find(" ") + 1;
			int q = s.length();
			s1 = s.substr(p, q-p);
			int j = 0;
			for(int i = 0; i < s1.length(); i++)
			{
				if(s1[i] == ' ')
				{
					j++;
				}
			}
			if(j == 1)
			{
				err = "";
				return true;
			}
			else
			{
				err = WRONG_NUMBER_FILE;
				return false;
			}
		}
		else
		{
			err = WRONG_FIELD;
			return false;
		}
	}
	else
	{
		err = WRONG_COMMAND;
		return false;
	}
}

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

	bzero(buffer, 1024);
	read(sock_fd, buffer, 1024);
	s = buffer;
	buffer[0] = '\0';
	bzero(buffer, 1024);
	cout << s << "\n";
	if(s.compare(SUCCESSFUL_CONNECTION) == 0)
	{
		s = "";
		getline(cin, s);

		while(s.compare("/exit") != 0)
		{
			string err = "";
			if(command_check(s, err))
			{
				// cout << "boooyay\n" << s;
				// break;
				buffer[0] = '\0';
				bzero(buffer, 1024);
				strcpy(buffer, s.c_str());
				n = send(sock_fd, buffer, s.length(), 0);
				buffer[0] = '\0';
				bzero(buffer, 1024);
				
				if(s.rfind("/sort", 0) == 0)
				{
					int count;
					string* str = split(s, ' ', count);
					char by = str[count - 1][0];
					if(count != 3 || (by != 'D' && by != 'N' && by != 'P'))
					{
						cout << WRONG_COMMAND << by << endl;
						goto L;
					}
					send_file(str[1], sock_fd);
					bzero(buffer, 1024);
					n = read(sock_fd,buffer,1024);
					s = buffer;
					buffer[0] = '\0';
					bzero(buffer, 1024);
					if(s.rfind("ERROR", 0) != 0)
					{
						cout << s << "\n";
						// break;
						rcv_file(sock_fd);
					}
					else
					{
						cout << s << "\n";
					}
				}

				if(s.rfind("/merge", 0) == 0)
				{
					int count;
					string* str = split(s, ' ', count);
					char by = str[count - 1][0];
					if(count != 5 || (by != 'D' && by != 'N' && by != 'P'))
					{
						cout << WRONG_COMMAND << by << endl;
						goto L;
					}

					send_file(str[1], sock_fd);
					send_file(str[2], sock_fd);
					
					bzero(buffer, 1024);
					n = read(sock_fd,buffer,1024);
					s = buffer;
					buffer[0] = '\0';
					bzero(buffer, 1024);

					if(s.rfind("ERROR", 0) != 0)
					{
						rcv_file(sock_fd);
					}
					else
						cout << s << "\n";
				}
				if(s.rfind("/similarity", 0) == 0)
				{
					int count;
					string* str = split(s, ' ', count);
					if(count != 3)
					{
						cout << WRONG_COMMAND << endl;
						goto L;
					}
					
					send_file(str[1], sock_fd);
					send_file(str[2], sock_fd);

					bzero(buffer, 1024);
					n = read(sock_fd,buffer,1024);
					s = buffer;
					buffer[0] = '\0';
					bzero(buffer, 1024);
					
					if(s.rfind("ERROR", 0) != 0)
					{
						cout << s << endl;
						rcv_file(sock_fd);
					}
					else
						cout << s << "\n";
				}
				L:s = "";
				getline(cin, s);
				// cout << s << "\n";
			}
			else
			{
				cout << err << "\n";
				getline(cin, s);
			}
		}
		close(sock_fd);
	}
	else if(s.compare(SERVER_BUSY) == 0)
	{
		cout << SERVER_BUSY << "\n";
	}

}