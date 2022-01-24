#include <iostream>
#include <fstream>

using namespace std;

int main()
{
	ifstream prF("test_price.txt"), strF("test_str.txt"), dateF("test_date.txt");
	ofstream fi("test_test.txt");
	string s, price, str, date;
	int count = 0;
	while(!(prF.eof() && strF.eof() && dateF.eof()))
	{
		if(!prF.eof())
			prF >> price;
		if(!strF.eof())
			strF >> str;
		if(!dateF.eof())
			dateF >> date;
		s = "";
		s = date + " " + str + " " + price;
		fi << s << endl;
		count++;
		if(count >= 500) break;
		
	}
	prF.close();
	strF.close();
	dateF.close();
	fi.close();
	// string s;
	// getline(cin, s);
	// if(s.substr(s.length()-4, s.length()).compare(".txt") == 0)
	// 	cout << "yes\n";
	// else
	// 	cout << "no\n";
	// int count = 0;
	// string* str = split(s, ' ', count);
	// for(int i = 0; i < count; i++)
	// {
	//     cout << str[i] << "\n";
	// }
}