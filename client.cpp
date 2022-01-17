#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#define PORT 5000

using namespace std;

int main()
{
    int sock_fd, n;
    char buffer[1024];
    string s;
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

    while(s.compare("exit") != 0)
    {
        n = write(sock_fd, s.c_str(), s.length());
        bzero(buffer, 1024);
        n = read(sock_fd,buffer,1024);
        s = buffer;
        cout << "server: " << s << "\n";
        getline(cin, s);
    }
    close(sock_fd);
}