#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 3010

int main(int argc, char *argv[]){
	/*
		Usage: ./a.out
	*/
	if (argc != 1){
		printf("Argument count should be 1.\nUsage: ./a.out\n");
		exit(EXIT_FAILURE);
	}

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


	int DATA = 0;
	if (send(sock_fd, DATA, sizeof(DATA), 0) == -1){
		perror("Sending data via socket");
		exit(EXIT_FAILURE);
	}

	return 0;
}