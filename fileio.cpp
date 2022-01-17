#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>

using namespace std;

#define ERR "kill me"

int main()
{
	int read_fd;
	int write_fd;
	struct stat stat_buf;
	off_t offset = 0;
	read_fd = open ("server_file.txt", O_RDONLY);
	fstat (read_fd, &stat_buf);
	write_fd = open ("server_file_temp.txt", O_WRONLY | O_CREAT, stat_buf.st_mode);
	sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
	cout << string(ERR).substr(5) << "\n";
	// FILE* fp;
	// char * line = NULL;
	// size_t len = 0;
	// ssize_t read;
	// int count = 0;
	// char lines[100] = "\nasdasjndjas sdajndaj\n";

	// fp = fopen("test.txt", "a+");
	// if(fp == NULL)
	// {
	// 	perror("error reading file");
	// 	exit(EXIT_FAILURE);
	// }

	// while((read = getline(&line, &len, fp)) != -1)
	// {
	// 	printf("len = %ld : %s", read, line);
	// 	count++;
	// 	if(count == 100) break;
	// }
	// ssize_t ofset = ftell(fp);
	// fclose(fp);
	// fp = fopen("test.txt", "a");
	// // printf("%ld", );
	// printf("\nno of lines: %d", count);

	// fseek(fp, ofset, SEEK_SET);

	// fputs(lines, fp);

}
