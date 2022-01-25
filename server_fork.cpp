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
#define WRONG_COMMAND "command not recognized"
#define INVALID_BILL "invalid records in bill"
#define NOT_SORTED_ALONG_FIELD "files aren't sorted along the field axis"
#define MAX_CHILD 2
#define MAX_VALID_YR 9999
#define MIN_VALID_YR 1800

using namespace std;

int Lines = -1;

// share memory operation
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
bool isLeap(int year)
{
	return (((year % 4 == 0) &&
		(year % 100 != 0)) ||
		(year % 400 == 0));
}

bool isValidDate(string date_str)
{
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
		regex reg("([0-3][0-9]\\.[0-1][0-2]\\.[1-2][0-9][0-9][0-9] \\S* \\d*\\.?\\d*)");
		if(!regex_match(s, reg))
		{
			validity = false;
			date = "";
			item = "";
			price = -1;
		}
		else
		{
			string* str = split(s, ' ', cnt);
			if(cnt != 3)
			{
				validity = false;
				date = "";
				item = "";
				price = -1;
			}
			else if(!isValidDate(date))
			{
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
				}
				catch(int x)
				{
					price = -1;
					validity = false;
				}
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
	// necessory line for checking validity of records and further the bill

	ifstream fi;
	fi.open(filename, ios::in);
	string s;
	record rec[count+5];
	count = 0;
	while(fi.eof())
	{
		getline(fi, s);
		rec[count] = record(s);
		if(!rec[count].isRecValid())
		{
			fi.close();
			return INVALID_BILL;
		}
		count++;
	}
	fi.close();
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
		fo << rec[i].giveString() << "\n";
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
					string filenameSend = "";
					// sorting
					if(str[0].compare("/sort") == 0)
					{
						cout << "here sorting\n";
						filename = str[1];
						ofstream of;
						int cont = 0;
						of.open(filename, ios::out);
						char by = s[s.length() - 1];
						bzero(buffer, 1024);
						read( new_sock , buffer, 1024);
						s = buffer;
						while(s.compare("eof") != 0)
						{
							of << s+"\n";
							cont++;
							bzero(buffer, 1024);
							read( new_sock , buffer, 1024);
							s = buffer;
						}
						of.close();
						filenameSend = filename;
						filename = sort_bills(filename, by, cont);
						if(filename.substr(filename.length()-4, filename.length()).compare(".txt"))
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
						filename = merge(str[1], str[2], str[count - 1][0]);
						if(filename.substr(filename.length()-4, filename.length()).compare(".txt"))
							passFile = true;
					}

					// similarity check
					
					else if(str[0].compare("/similarity") == 0)
					{
						string filename1 = str[1];
						string filename2 = str[2];

						ofstream of1, of2;
						of1.open(filename1, ios::out);
						of2.open(filename2, ios::out);
						
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
						filename = similarity(filename1, filename2);
						if(filename.substr(filename.length()-4, filename.length()).compare(".txt"))
							passFile = true;
					}
					if(!passFile)
					{
						send(new_sock, filename.c_str(), filename.length(), 0);
					}
					else
					{
						send(new_sock, filenameSend.c_str(), filename.length(), 0);
						int i = 0;
						ifstream fi(filename);
						while(!fi.eof())
						{
							s = "";
							getline(fi, s);
							send(new_sock, s.c_str(), s.length(), 0);
						}
						s = "eof";
						send(new_sock, s.c_str(), s.length(), 0);
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