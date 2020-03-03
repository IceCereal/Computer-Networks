#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#include "common.h"

#define PORT 3010
#define BUFFER 1024

int main(int argc, char *argv[]){
	/*
		Usage: ./a.out
	*/
	if (argc != 1){
		printf("Argument count should be 1.\nUsage: ./a.out\n");
		exit(EXIT_FAILURE);
	}

	char filename[256];
	struct stat file_stats;

	printf("Enter Filename:\t");
	scanf("%s", filename);

	int fd = open(filename, O_RDONLY);
	if (fstat(fd, &file_stats) < 0){
		printf("FAIL TO READ FILE!");
		exit(EXIT_FAILURE);
	}
	close(fd);

	int sock_fd;
	struct sockaddr_in servaddr;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket Failed!");
		exit(EXIT_FAILURE);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0){
		perror("Invalid Address!");
		exit(EXIT_FAILURE);
	}

	if (connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Connection Failed!");
		exit(EXIT_FAILURE);
	}

	struct Head head;

	strcpy(head.FILENAME, filename);
	
	FILE *filePtr = fopen(head.FILENAME, "rb");

	fseek(filePtr, 0, SEEK_END);
	int fsize = (int) ftell(filePtr);
	fseek(filePtr, 0, SEEK_SET);

	head.length = fsize;

	printf("Sending File Metadata...\nFile:\t%s\nFile Size:\t%d\n", head.FILENAME, head.length);

	if (send(sock_fd, &head, sizeof(head), 0) == -1){
		perror("Sending data via socket");
		exit(EXIT_FAILURE);
	}

	char buffer[fsize];

	fread(buffer, 1, fsize, filePtr);

	printf("%ld", send(sock_fd, buffer, fsize, 0));

	return 0;
}