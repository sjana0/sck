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
#define WRONG_FIELD "wrong field axis"

using namespace std;

/***
only sends the content of the file
so has filename
and sock
***/

void send_file(string filename, int sock)
{
	cout << "inside send function1\n";
	char buffer[1024];
	string s;

	// wirte filename to server
	ifstream fi(filename);
	write(sock, filename.c_str(), filename.length());
	
	cout << "inside send function2\n";
	while(!fi.eof())
	{
		// it reads first ack for filename from server
		bzero(buffer, 1024);
		read(sock, buffer, 1024);
		s = buffer;
		cout << "1: " << s << "\n";
		
		if(s.compare("acknowledged") == 0)
		{
			// cout << "inside send function3\n" << s << "\n";
			// reads first line from file and sends
			getline(fi, s);
			cout << "2: " << s << "\n";
			write(sock, s.c_str(), s.length());
			cout << "3: " << s << "\n";
		}
	}
	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	s = buffer;
	if(s.compare("acknowledged") == 0)
	{
		s = "eof";
		write(sock, s.c_str(), s.length());
	}
}

/***
filename send from server and sock
***/

string rcv_file(int sock)
{
	cout << "recieve file\n";
	char buffer[1024];
	string s;
	
	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	string filename = buffer;
	cout << "recieve file\n";
	
	ofstream fo(filename);
	
	s = "acknowledged";
	write(sock, s.c_str(), s.length());
	
	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	s = buffer;
	
	string s1;
	while(s.compare("eof") != 0)
	{
		s1 = s;
		s = "acknowledged";
		write(sock, s.c_str(), s.length());
		bzero(buffer, 1024);
		read(sock, buffer, 1024);
		s = buffer;
		cout << s1 << "\n";
		if(s.compare("eof") == 0)
			fo << s1;
		else
			fo << s1 << endl;
	}
	return filename;
}

// bool ifFileExists (const std::string& name) {
// 	struct stat buffer;
// 	return (stat (name.c_str(), &buffer) == 0);
// }

// bool command_check(string s, string& err)
// {
// 	if(s.rfind("/sort", 0) == 0 || s.rfind("/merge", 0) == 0 || s.rfind("/similarity", 0) == 0)
// 	{
// 		if(s.rfind("/similarity", 0) == 0 || ((s.rfind("/sort", 0) == 0 || s.rfind("/merge", 0) == 0) && (s[s.length()-1] == 'D' || s[s.length()-1] == 'N' || s[s.length()-1] == 'P')))
// 		{
// 			if(s.rfind("/sort", 0) == 0 || s.rfind("/merge", 0) == 0)
// 				s = s.substr(s.find(" ") + 1, s.rfind(" "));
// 			else
// 				s = s.substr(s.find(" ") + 1, s.length());
// 			int j = 0;
// 			for(int i = 0; i < s.length(); i++)
// 			{
// 				if(s[i] == ' ')
// 				{
// 					if(!ifFileExists(s.substr(j, i)))
// 					{
// 						err = FILE_NOT_EXISTS;
// 						return false;
// 					}
// 					j = i+1;
// 				}
// 			}
// 			err = "";
// 			return true;
// 		}
// 		else
// 		{
// 			err = WRONG_FIELD;
// 			return false;
// 		}
// 	}
// 	else
// 	{
// 		err = WRONG_COMMAND;
// 		return false;
// 	}
// }

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
			// cout << "boooyay\n" << s;
			// break;
			n = write(sock_fd, s.c_str(), s.length());
			
			if(s.rfind("/sort", 0) == 0)
			{
				cout << "sorting\n";
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
				if(count != 4 || (by != 'D' && by != 'N' && by != 'P'))
				{
					cout << WRONG_COMMAND << by << endl;
					goto L;
				}
				for(int i = 0; i < count-2; i++)
				{
					send_file(str[i+1], sock_fd);
				}
				
				bzero(buffer, 1024);
				n = read(sock_fd,buffer,1024);
				s = buffer;
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
			// cout << err << "\n";
			getline(cin, s);
		}
	}
	close(sock_fd);
}