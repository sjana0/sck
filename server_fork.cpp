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
#define MAX_CHILD 2

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

class Date
{
public:
	string dateStr;
	int day, month, year;

	const int max_valid_yr = 9999, min_valid_yr = 1800

	Date(string date_str)
	{
		dateStr = date_str;
		int cnt;
		string *str = split(date_str, '.', cnt);
		day = stoi(str[0]);
		month = stoi(str[1]);
		year = stoi(str[2]);
	}

	Date(string date_str, int d, int m, int y)
	{
		dateStr = date_str;
		day = d;
		month = m;
		year = y;
	}

	Date(Date date)
	{
		this->dateStr = date.dateStr;
		this->day = date.day;
		this->month = date.month;
		this->year = date.year;
	}


	bool operator==(Date lhs, Date rhs)
	{
		if(lhs.day == rhs.day && lhs.month == rhs.month && lhs.year == rhs.year)
			return true;
		else
			false;
	}

	bool operator>(Date lhs, Date rhs)
	{
		if(lhs.year > rhs.year)
			return true;
		else if(lhs.year < rhs.year)
			return false;
		else
		{
			if(lhs.month > rhs.month)
				return true;
			else if(lhs.month < rhs.month)
				return false;
			else
			{
				if(lhs.day > rhs.day)
					return true;
				return false;
			}
		}
	}

	bool operator<(Date lhs, Date rhs)
	{
		if(lhs.year < rhs.year)
			return true;
		else if(lhs.year > rhs.year)
			return false;
		else
		{
			if(lhs.month < rhs.month)
				return true;
			else if(lhs.month > rhs.month)
				return false;
			else
			{
				if(lhs.day < rhs.day)
					return true;
				return false;
			}
		}
	}

	bool isLeap(int year)
	{
		return (((year % 4 == 0) &&
			(year % 100 != 0)) ||
			(year % 400 == 0));
	}

	bool isValidDate()
	{
		if (year > max_valid_yr || year < min_valid_yr)
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

	int dateCompare(Date dt)
	{
		if(*this > dt)
			return 1;
		else if(*this == dt)
			return 0;
		else
			return -1;
	}
};

class record
{
public:
	string date, item;
	double price;
	bool validity;

	record(string s)
	{
		int cnt;
		string* str = split(s, ' ', cnt);
		date = str[0];
		item = str[1];
		validity = true;
		if(Date(date).isValidDate())
		{
			validity = false;
		}
		try
		{
			price = stod(str[2]);
		}
		catch(Exception)
		{
			price = -1;
			validity = false;
		}
	}

	record(record rec)
	{
		date = rec.date;
		item = rec.item;
		price = rec.price;
	}

	bool isRecValid()
	{
		return validity;
	}

	int recCompare(record rhs, char by)
	{
		if(by == 'D')
		{
			Date dt1 = Date(this->date);
			Date dt2 = Date(rhs.date);
			if(dt1 == dt2)
				return 0;
			else if(dt1 > dt2)
				return 1;
			else
				return -1;
		}
		else if(by == 'N')
		{
			return this->item.compare(rhs.item);
		}
		else if(by == 'P')
		{
			if(this->price == rhs.price)
				return 0;
			else if(this->price > rhs.price)
				return 1;
			else
				return -1;
		}
	}

	void swap_recs(record& bil1, record& bil2)
	{
		string date = bil1.date, item = bil1.item;
		double price = bil1.price;
		bil1.date = bil2.date;
		bil1.item = bil2.item;
		bil1.price = bil2.price;
		bil2.date = date;
		bil2.item = item;
		bil2.price = price;
	}
};

class bill
{
public:
	record *arr;
	int n;
	bill(int n)
	{
		this->n = n;
		arr = new record[n];
	}
	void sort_bills(char by)
	{
		if(by == 'D')
		{

		}
		else if(by == 'N')
		{

		}
		else if(by == 'P')
		{

		}
	}
	void print()
	{
		for(int i = 0; i < n; i++)
		{
			cout << arr[i].date << " " << arr[i].item << " " << arr[i].price << "\n";
		}
	}
};

// share memory for multiprocess operations
// int shmid = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT);

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

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	// Forcefully attaching socket to the port 5000
	if (bind(sock_fd, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(sock_fd, 1) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in cli_addr;
	socklen_t clilen;
	clilen = sizeof(cli_addr);
	bool b = false;
	while(1)
	{
		if(a[0] != 0)
		{
			if ((new_sock = accept(sock_fd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			a[0]--;
			p = fork();
		}
		if(p)
		{
			// a[0]--;
			while(a[0] == 0)
			{
				// sleep(3);
				cout << "here " << a[0] << "\n";
			}
		}
		else
		{
			// child process: process command from client
			close(sock_fd);
			// a[0]--;
			cout << "new " << a[0] << "\n";
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
			return 0;
			// child process
		}
	}

}