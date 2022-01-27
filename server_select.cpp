#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <regex>
#define PORT 5000

// server messages
#define WRONG_COMMAND "ERROR: command not recognized\n"
#define INVALID_BILL "ERROR: invalid records in bill\n"
#define NOT_SORTED_ALONG_FIELD "ERROR: files aren't sorted along the field axis\n"
#define SUCCESSFUL_CONNECTION "successful connection"
#define SERVER_BUSY "unsuccessful connection server is busy(MAX 5 client reached)"
#define MAX_CLIENTS 2
#define MAX_VALID_YR 9999
#define MIN_VALID_YR 1800

using namespace std;

void remove_empty_line(string filename)
{
	ifstream fin(filename);
	ofstream fout("temp.txt");
	string rep, rep1;
	getline(fin, rep);
	while(!fin.eof())
	{
		rep1 = rep;
		getline(fin, rep);
		// cout << "rep1: " << rep1 << " rep: " << rep << "\n";
		if(rep.compare("") == 0)
			fout << rep1;
		else
			fout << rep1 << endl;
	}
	fout.close();
	fin.close();
	remove(filename.c_str());
	rename("temp.txt", filename.c_str());
}

/***
send file will have the file name that have to be sent
the name of the file in server side that has to be sent
and finally the sock file descriptor
***/

void send_file(string filename, string filenameSend, int sock)
{
	char buffer[1024];
	string s;
	ifstream fi(filename);
	strcpy(buffer, filenameSend.c_str());
	write(sock, buffer, filenameSend.length());
	buffer[0] = '\0';
	bzero(buffer, 1024);
	
	while(!fi.eof())
	{
		buffer[0] = '\0';
		bzero(buffer, 1024);
		read(sock, buffer, 1024);
		s = buffer;
		if(s.compare("acknowledged") == 0)
		{
			bzero(buffer, 1024);
			getline(fi, s);
			strcpy(buffer, s.c_str());
			write(sock, buffer, s.length());
			buffer[0] = '\0';
		}
	}

	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	s = buffer;
	
	if(s.compare("acknowledged") == 0)
	{
		buffer[0] = '\0';
		bzero(buffer, 1024);
		s = "eof";
		strcpy(buffer, s.c_str());
		write(sock, buffer, s.length());
		buffer[0] = '\0';
		bzero(buffer, 1024);
	}
}

/***
recieve file will have
as filename already have been recieved so filename
count as to count the number lines in the file
and finally the sock fd
***/

string rcv_file(int sock, int& count)
{
	char buffer[1024];
	string s;
	string filename;
	count = 0;

	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	
	filename = buffer;
	filename = "server_" + filename;
	ofstream fo(filename);
	
	// send ack on recieving filename
	buffer[0] = '\0';
	bzero(buffer, 1024);
	s = "acknowledged";
	strcpy(buffer, s.c_str());
	write(sock, buffer, s.length());
	
	// reads the first line of the file
	buffer[0] = '\0';
	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	
	s = buffer;
	string s1;

	while(s.compare("eof") != 0)
	{
		s1 = s;

		// first line ack
		buffer[0] = '\0';
		bzero(buffer, 1024);
		s = "acknowledged";
		strcpy(buffer, s.c_str());
		write(sock, buffer, s.length());
		
		buffer[0] = '\0';
		bzero(buffer, 1024);
		read(sock, buffer, 1024);
		s = buffer;
		
		cout << "line: " << s1 << "\n";
		count++;
		if(s.compare("eof") == 0)
		{
			fo << s1;
			cout << "eof\n";
		}
		else
		{
			fo << s1 << endl;
		}
	}
	buffer[0] = '\0';
	bzero(buffer, 1024);
	fo.close();
	return filename;
}

string* split(string s,char c, int& count) {
	for(unsigned int p = 0; p < s.length(); p++) {
		if(s[p] == c) {
			count++;
		}
	}

	count++;
	string* strar = new string[count];
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

bool isLeap(int year)
{
	return (((year % 4 == 0) &&
		(year % 100 != 0)) ||
		(year % 400 == 0));
}

bool isValidDate(string date_str)
{
	// cout << "error here\n" << date_str << "\n";
	int year = stoi(date_str.substr(date_str.length()-4, date_str.length()));
	int month = stoi(date_str.substr(3, 5));
	int day = stoi(date_str.substr(0,2));
	if (year > MAX_VALID_YR || year < MIN_VALID_YR)
		return false;
	if (month < 1 || month > 12)
		return false;
	if (day < 1 || day > 31)
		return false;

	if (month == 2)
	{
		if (isLeap(year))
			return (day <= 29);
		else
			return (day <= 28);
	}

	if (month == 4 || month == 6 || month == 9 || month == 11)
		return (day <= 30);

	return true;
}

int compareDate(string d1, string d2)
{
	int cont;
	string* d11 = split(d1, '.', cont);
	d1 = "";
	d1 = d11[2] + d11[1] + d11[0];
	string* d22 = split(d2, '.', cont);
	d2 = "";
	d2 = d22[2] + d22[1] + d22[0];
	return d1.compare(d2);
}

class record
{
public:
	string date, item;
	double price;
	bool validity;
	
	record() {}

	record(string s)
	{
		int cnt;
		validity = true;
		cout << "record inside class: " << s << "\n";
		string* str = split(s, ' ', cnt);
		date = str[0];
		item = str[1];
		if(cnt != 3)
		{
			cout << "here invalidated2\n";
			validity = false;
			date = "";
			item = "";
			price = -1;
		}
		else if(!isValidDate(date))
		{
			cout << "here invalidated3\n";
			validity = false;
			date = "";
			price = -1;
		}
		else
		{
			try
			{
				date = str[0];
				item = str[1];
				price = stod(str[2]);
				cout << "ok\n";
			}
			catch(int x)
			{
				cout << "here invalidated4\n";
				price = -1;
				validity = false;
			}
		}
	}

	bool isRecValid()
	{	
		return validity;
	}

	string giveString()
	{
		string s = "";
		s = date + " " + item + " " + to_string(price);
		return s;
	}

	bool isSimilar(record& rec)
	{
		if(this->date.compare(rec.date) == 0 || this->item.compare(rec.item) == 0 || this->price == rec.price)
			return true;
		else
			return false;
		
	}

	int recCompare(record& rec, char by)
	{
		if(by == 'D')
		{
			return compareDate(this->date, rec.date);
		}
		else if(by == 'N')
		{
			return this->item.compare(rec.item);
		}
		else if(by == 'P')
		{
			return ((this->price - rec.price) < 0) ? -1 : (((this->price - rec.price) > 0) ? 1 : 0);
		}
	}
};

bool SrecCompareD(record &lhs, record &rhs)
{
	return (compareDate(lhs.date, rhs.date) < 0);
}

bool recCompareD(record &lhs, record &rhs)
{
	return (compareDate(lhs.date, rhs.date) <= 0);
}

bool SrecCompareN(record &lhs, record &rhs)
{
	return (lhs.item.compare(rhs.item) < 0);
}

bool recCompareN(record &lhs, record &rhs)
{
	return (lhs.item.compare(rhs.item) <= 0);
}

bool SrecCompareP(record &lhs, record &rhs)
{
	return (lhs.price < rhs.price);
}

bool recCompareP(record &lhs, record &rhs)
{
	return (lhs.price <= rhs.price);
}

bool isSorted(string filename, char by)
{
	bool isSorted = true;
	ifstream fi(filename);
	string s1, s2;

	getline(fi, s1);
	while(!fi.eof())
	{
		getline(fi, s2);
		record rec1(s1);
		record rec2(s2);
		if(by == 'D')
		{
			isSorted = recCompareD(rec1, rec2);
		}
		else if(by == 'N')
		{
			isSorted = recCompareN(rec1, rec2);
		}
		else if(by == 'P')
		{
			isSorted = recCompareP(rec1, rec2);
		}
		s1 = s2;
		if(!isSorted)
		{
			cout << "it's this: " << rec1.giveString() << " | " << rec2.giveString() << "\n";
			break;
		}
	}
	return isSorted;
}

bool isValidBill(string filename)
{
	ifstream fi(filename);
	string s;
	bool valid = true;
	while(!fi.eof())
	{
		getline(fi, s);
		if(!record(s).isRecValid())
		{
			valid = false;
			break;
		}
	}
	fi.close();
	return valid;
}


string sort_bills(string filename, char by, int count)
{

	ifstream fi(filename);
	// fi.open(filename, ios::in);
	string s;
	record rec[count+5];
	cout << "sort print " << filename << " by " << by << "\n";
	count = 0;
	// return "ERROR:";
	while(!fi.eof())
	{
		getline(fi, s);
		cout << s << "\n";
		rec[count] = record(s);
		if(!rec[count].isRecValid())
		{
			fi.close();
			return INVALID_BILL;
		}
		count++;
		cout << "inside sortbills function: " << s << "\n";
	}
	fi.close();
	// return "ERROR:";
	if(by == 'D')
	{
		sort(rec, rec+count, SrecCompareD);
		cout << "reached here-1: " << filename << "\n";
	}
	else if(by == 'N')
	{
		sort(rec, rec+count, SrecCompareN);
	}
	else if(by == 'P')
	{
		sort(rec, rec+count, SrecCompareP);
	}
	
	ofstream fo(filename);
	for(int i = 0; i < count; i++)
	{
		if(i != count-1)
			fo << rec[i].giveString() << "\n";
		else
			fo << rec[i].giveString();
	}
	cout << "reached here0: " << filename << "\n";
	fo.close();
	cout << "reached here1: " << filename << "\n";
	return filename;
}

string merge(string filename1, string filename2, char by)
{

	string filename = filename1.substr(0, filename1.length() - 4) + "_merge" + ".txt";
	ofstream fo(filename);
	int count = 0;
	if(isValidBill(filename1) && isValidBill(filename2))
	{
		if(isSorted(filename1, by) && isSorted(filename2, by))
		{
			ifstream fi1(filename1), fi2(filename2);
			string s;
			while(!fi1.eof())
			{
				getline(fi1, s);
				fo << s << endl;
				count++;
			}
			fi1.close();
			while(!fi2.eof())
			{
				getline(fi2, s);
				fo << s << endl;
				count++;
			}
			fi2.close();
		}
		else
		{
			fo.close();
			remove(filename.c_str());
			return NOT_SORTED_ALONG_FIELD;
		}
	}
	else
	{
		fo.close();
		remove(filename.c_str());
		return INVALID_BILL;
	}
	fo.close();
	remove_empty_line(filename);
	cout << "reached hear2: "<< filename << "\n";
	// sleep(2);
	sort_bills(filename, by, count);
	cout << "reached hear3: "<< filename << "\n";
	return filename;
}

string similarity(string filename1, string filename2)
{
	string filename = filename1.substr(0, filename1.length() - 4) + "_sim_" + filename2;
	if(isValidBill(filename1) && isValidBill(filename2))
	{
		ofstream fo(filename);
		ifstream fi1(filename1), fi2(filename2);
		string s1, s2;
		while(!fi1.eof())
		{
			getline(fi1, s1);
			while(!fi2.eof())
			{
				getline(fi2, s2);
				record rec1(s1);
				record rec2(s2);
				if(rec1.isSimilar(rec2))
					fo << s1 + " | " + s2 << endl;
			}
			fi2.clear();
			fi2.seekg(0);
		}
		fi1.close();
		fi2.close();
		cout << "calling out the function\n";
		remove_empty_line(filename);
		return filename;
	}
	else
	{
		remove(filename.c_str());
		return INVALID_BILL;
	}
}

int main(int argc , char *argv[])
{
	int opt = true;
	int master_socket , addrlen , new_socket , client_socket[MAX_CLIENTS] , max_clients = MAX_CLIENTS , activity, i , valread , sd;
	int max_sd;
	struct sockaddr_in address;
	 
	char buffer[1025];  //data buffer of 1K
	 
	//set of socket descriptors
	fd_set readfds;
	 
	//a message
	// char *message = SUCCESSFUL_CONNECTION;
 
	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++) 
	{
		client_socket[i] = 0;
	}
	 
	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
 
	//set master socket to allow multiple connections , this is just a good habit, it will work without this
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
 
	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	 
	//bind the socket to localhost port 5000
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener on port %d \n", PORT);
	
	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	 
	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");
	
	while(true) 
	{
		//clear the socket set
		FD_ZERO(&readfds);
 
		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		
		//add child sockets to set
		for ( i = 0 ; i < max_clients ; i++) 
		{
			//socket descriptor
			sd = client_socket[i];
			
			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
			
			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}
 
		//wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
   
		if ((activity < 0) && (errno!=EINTR)) 
		{
			printf("select error");
		}
		 
		//If something happened on the master socket , then its an incoming connection
		if (FD_ISSET(master_socket, &readfds)) 
		{
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
		 
			//inform user of socket number - used in send and receive commands
			printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
	   
			//send new connection greeting message
			// if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
			// {
			// 	perror("send");
			// }
			 
			puts("Welcome message sent successfully");
			 
			//add new socket to array of sockets
			string msg = SERVER_BUSY;
			for (i = 0; i < max_clients; i++) 
			{
				//if position is empty
				if( client_socket[i] == 0 )
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
					msg = SUCCESSFUL_CONNECTION;
					break;
				}
			}

			buffer[0] = '\0';
			bzero(buffer, 1024);
			strcpy(buffer, msg.c_str());
			send(new_socket, buffer, msg.length(), 0);
			buffer[0] = '\0';
			bzero(buffer, 1024);
		}
		 
		//else its some IO operation on some other socket :)
		for (i = 0; i < max_clients; i++) 
		{
			sd = client_socket[i];
			 
			if (FD_ISSET( sd , &readfds)) 
			{
				//Check if it was for closing , and also read the incoming message
				if ((valread = read( sd , buffer, 1024)) == 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
					 
					//Close the socket and mark as 0 in list for reuse
					close( sd );
					client_socket[i] = 0;
				}
				 
				//Echo back the message that came in
				else
				{
					//set the string terminating NULL byte on the end of the data read
					buffer[valread] = '\0';
					string s = buffer;
					cout << s << "\n";
					// break;

					bool passFile = false;
					while(valread > 0 && s.compare("/exit") != 0) {
						passFile = true;
						int count = 0;
						string* str = split(s, ' ', count);
						string filename = "";
						string filenameSend = str[1];
						cout << "command: " << s << "\n";
						// sorting
						if(str[0].compare("/sort") == 0)
						{
							char by = str[2][0];
							cout << s << "\n" << by << "\n";
							int cont = 0;
							filename = rcv_file(sd, cont);
							filename = sort_bills(filename, by, cont);
							if((!filename.rfind("ERROR:", 0) == 0) && (filename.rfind(".txt") == filename.length()-4))
								passFile = true;
						}
						// merge

						else if(str[0].compare("/merge") == 0)
						{
							string filename1, filename2;
							filename1 = str[1];
							filename2 = str[2];
							filenameSend = str[3];
							int cont;

							filename1 = rcv_file(sd, cont);
							filename2 = rcv_file(sd, cont);

							filename = merge(filename1, filename2, str[count - 1][0]);

							remove(filename1.c_str());
							remove(filename2.c_str());

							if((!filename.rfind("ERROR:", 0) == 0) && (filename.rfind(".txt") == filename.length()-4))
								passFile = true;
						}

						// similarity check
						
						else if(str[0].compare("/similarity") == 0)
						{
							string filename1 = str[1];
							string filename2 = str[2];
							int cont = 0;

							filenameSend = filename1.substr(0, filename1.length() - 4) + "_sim_" + filename2;
							cout << "filename1: " << filename1 << "filename2: " << filename2 << "filenameSend: " << filenameSend << "\n";
							
							filename1 = rcv_file(sd, cont);
							filename2 = rcv_file(sd, cont);
							
							filename = similarity(filename1, filename2);

							remove(filename1.c_str());
							remove(filename2.c_str());
							
							if((!filename.rfind("ERROR:", 0) == 0) && (filename.rfind(".txt") == filename.length()-4))
								passFile = true;
						}
						if(!passFile)
						{
							bzero(buffer, 1024);
							strcpy(buffer, filename.c_str());
							send(sd, buffer, filename.length(), 0);
							buffer[0] = '\0';
							bzero(buffer, 1024);
							send(sd, filename.c_str(), filename.length(), 0);
						}
						else
						{
							cout << "outside passfile: "<<filename <<"\n";
							s = "Successful command\n";
							buffer[0] = '\0';
							bzero(buffer, 1024);
							strcpy(buffer, s.c_str());
							send(sd, buffer, s.length(), 0);
							buffer[0] = '\0';
							bzero(buffer, 1024);
							send_file(filename, filenameSend, sd);
							remove(filename.c_str());
						}
						// break;
						buffer[0] = '\0';
						bzero(buffer, 1024);
						valread = read( sd , buffer, 1024);
						s = buffer;
						buffer[0] = '\0';
						bzero(buffer, 1024);
						cout << s << "\n";
					}
					close(sd);
					// child process
				}
			}
		}
	}
	 
	return 0;
}