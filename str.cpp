#include <iostream>

using namespace std;

string* split(string s,char c, int& count) {
	string static strar[100];
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
    string s;
    getline(cin, s);
    int count = 0;
    string* str = split(s, ' ', count);
    for(int i = 0; i < count; i++)
    {
        cout << str[i] << "\n";
    }
}