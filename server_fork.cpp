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
#include <fstream>
#include <regex>
#define PORT 5000

// server messages
#define WRONG_COMMAND "ERROR: command not recognized\n"
#define INVALID_BILL "ERROR: invalid records in bill\n"
#define NOT_SORTED_ALONG_FIELD "ERROR: files aren't sorted along the field axis\n"
#define MAX_CHILD 2
#define MAX_VALID_YR 9999
#define MIN_VALID_YR 1800

using namespace std;

int Lines = -1;

// share memory operation
int shmid = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT);

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
	write(sock, filenameSend.c_str(), filenameSend.length());
	
	while(!fi.eof())
	{
		bzero(buffer, 1024);
		read(sock, buffer, 1024);
		s = buffer;
		if(s.compare("acknowledged") == 0)
		{
			getline(fi, s);
			write(sock, s.c_str(), s.length());
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
	s = "acknowledged";
	write(sock, s.c_str(), s.length());
	
	// reads the first line of the file
	bzero(buffer, 1024);
	read(sock, buffer, 1024);
	
	s = buffer;
	string s1;

	while(s.compare("eof") != 0)
	{
		s1 = s;

		// first line ack
		s = "acknowledged";
		write(sock, s.c_str(), s.length());
		
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
		regex reg("([0-3][0-9]\\.[0-1][0-2]\\.[1-2][0-9][0-9][0-9] \\S* \\d*\\.?\\d*)");
		// if(!regex_match(s, reg))
		// {
		//	cout << "in regex match\n";
		// 	cout << "here invalidated1\n";
		// 	validity = false;
		// 	date = "";
		// 	item = "";
		// 	price = -1;
		// }
		// else
		// {
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
		// }
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

bool recCompareD(record &lhs, record &rhs)
{
	return (compareDate(lhs.date, rhs.date) < 0);
}

bool recCompareN(record &lhs, record &rhs)
{
	return (lhs.item.compare(rhs.item) < 0);
}

bool recCompareP(record &lhs, record &rhs)
{
	return (lhs.price < rhs.price);
}

bool isSorted(string filename, char by)
{
	bool isSorted = true;
	ifstream fi(filename);
	string s1, s2;

	while(!fi.eof())
	{
		getline(fi, s1);
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
		if(!isSorted)
			break;
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

	ifstream fi;
	fi.open(filename, ios::in);
	string s;
	record rec[count+5];
	cout << "sort print " << by << "\n";
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
		cout << "inside sortbills function: " << s;
	}
	fi.close();
	// return "ERROR:";
	if(by == 'D')
	{
		sort(rec, rec+count, recCompareD);
	}
	else if(by == 'N')
	{
		sort(rec, rec+count, recCompareN);
	}
	else if(by == 'P')
	{
		sort(rec, rec+count, recCompareP);
	}
	
	ofstream fo(filename);
	for(int i = 0; i < count; i++)
	{
		if(i != count-1)
			fo << rec[i].giveString() << "\n";
		else
			fo << rec[i].giveString();
	}
	fo.close();
	return filename;
}

string merge(string filename1, string filename2, char by)
{
	if(isValidBill(filename1) && isValidBill(filename2))
	{
		string filename = filename1.substr(0, filename1.length() - 4) + "_merge" + ".txt";
		if(isSorted(filename1, by) && isSorted(filename2, by))
		{
			ifstream fi1(filename1), fi2(filename2);
			ofstream fo(filename);
			string s;
			while(!fi1.eof())
			{
				getline(fi1, s);
				fo << s << endl;
			}
			while(!fi2.eof())
			{
				getline(fi2, s);
				fo << s << endl;
			}
			fi1.close();
			fi2.close();
			fo.close();
			return filename;
		}
		else
			return NOT_SORTED_ALONG_FIELD;
	}
	else
		return INVALID_BILL;
}

string similarity(string filename1, string filename2)
{
	string filename = filename1.substr(0, filename1.length() - 4) + "_sim" + ".txt";
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
		return filename;
	}
	else
		return INVALID_BILL;
}


int main()
{
	// initialize shared memory
	int *a;
	a = (int*)shmat(shmid, 0, 0);
	a[0] = MAX_CHILD;

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

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
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

	if (listen(sock_fd, 2) < 0)
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

			if(p)
			{
				// close(new_sock);
				if(a[0] != 0)
				{
					a[0]--;
					cout << "client new "<<a[0] << "\n";
				}
				
			}
			else
			{
				// child process: process command from client
				close(sock_fd);

				valread = read( new_sock , buffer, 1024);
				string s(buffer);

				bool passFile = false;
				while(valread > 0 && s.compare("/exit") != 0) {
					int count = 0;
					string* str = split(s, ' ', count);
					string filename = "";
					string filenameSend = str[1];
					// sorting
					if(str[0].compare("/sort") == 0)
					{
						char by = str[2][0];
						cout << s << "\n" << by << "\n";
						int cont = 0;
						filename = rcv_file(new_sock, cont);
						filename = sort_bills(filename, by, cont);
						if(filename.substr(filename.length()-4, filename.length()).compare(".txt") == 0)
							passFile = true;
					}
					// merge

					else if(str[0].compare("/merge") == 0)
					{
						for(int i = 0; i < count-2; i++)
						{
							ofstream of;
							of.open(str[i+1]);
							bzero(buffer, 1024);
							read( new_sock , buffer, 1024);
							s = buffer;
							while(s.compare("eof") != 0)
							{
								of << s+"\n";
								bzero(buffer, 1024);
								read( new_sock , buffer, 1024);
								s = buffer;
							}
							of.close();
						}
						filenameSend = str[1];
						filename = merge("server_" + str[1], "server_" + str[2], str[count - 1][0]);
						if(filename.substr(filename.length()-4, filename.length()).compare(".txt") == 0)
							passFile = true;
					}

					// similarity check
					
					else if(str[0].compare("/similarity") == 0)
					{
						string filename1 = str[1];
						string filename2 = str[2];

						ofstream of1, of2;
						of1.open("server_" + filename1, ios::out);
						of2.open("server_" + filename2, ios::out);
						
						// read 1st file
						
						bzero(buffer, 1024);
						read( new_sock , buffer, 1024);
						s = buffer;
						while(s.compare("eof") != 0)
						{
							of1 << s+"\n";
							bzero(buffer, 1024);
							read( new_sock , buffer, 1024);
							s = buffer;
						}

						// read 2nd file
						bzero(buffer, 1024);
						read( new_sock , buffer, 1024);
						s = buffer;
						while(s.compare("eof") != 0)
						{
							of2 << s+"\n";
							bzero(buffer, 1024);
							read( new_sock , buffer, 1024);
							s = buffer;
						}
						of1.close();
						of2.close();
						filename = similarity("server_" + filename1, "server_" + filename2);
						if(filename.substr(filename.length()-4, filename.length()).compare(".txt") == 0)
							passFile = true;
					}
					if(!passFile)
					{
						send(new_sock, filename.c_str(), filename.length(), 0);
					}
					else
					{
						cout << "outside passfile: "<<filename <<"\n";
						s = "Successful command\n";
						send(new_sock, s.c_str(), s.length(), 0);
						send_file(filename, filenameSend, new_sock);
					}
					bzero(buffer, 1024);
					valread = read( new_sock , buffer, 1024);
					s = buffer;
					cout << s << "\n";
				}
				close(new_sock);
				a[0]++;
				cout << "making shm leaving " << a[0] << "\n";
				break;
				// child process
			}
		}
		else
		{
			cout << "doing it\n";
			close(sock_fd);
			while(a[0] == 0);
			if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
			{
				perror("socket failed");
				exit(EXIT_FAILURE);
			}

			if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
			{
				perror("setsockopt");
				exit(EXIT_FAILURE);
			}

			// Forcefully attaching socket to the port 5000
			if (bind(sock_fd, (struct sockaddr *)&address, sizeof(address))<0)
			{
				perror("bind failed");
				exit(EXIT_FAILURE);
			}

			if (listen(sock_fd, 2) < 0)
			{
				perror("listen");
				exit(EXIT_FAILURE);
			}
		}
	}
}