#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"

#define PORT 3010
#define BUFFER 1024
#define BACKLOG 4

int main(int argc, char *argv[]){
	int listen_fd, conn_fd;

	struct sockaddr_in servaddr;
	int servaddrlen = sizeof(servaddr);

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("Socket Failed!");
		exit(EXIT_FAILURE);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Bind Failed!");
		exit(EXIT_FAILURE);
	}

	if (listen(listen_fd, BACKLOG) < 0){
		perror("Listen Failed!");
		exit(EXIT_FAILURE);
	}

	printf("Local IP address : %s \n", inet_ntoa(servaddr.sin_addr));

	struct sockaddr cliaddr;

	for (;;){
		if ((conn_fd = accept(listen_fd, (struct sockaddr *)&cliaddr, (socklen_t *)&servaddrlen)) < 0){
			perror("Accepting Connection!");
			exit(EXIT_FAILURE);
		}
		
		printf("\nGot connection!\n");

		struct Head head;

		if (recv(conn_fd, &head, sizeof(head), 0) == -1){
			perror("Error in receiving data!");
			exit(EXIT_FAILURE);
		}

		printf("Receiving File:\treceived/%s\n", head.FILENAME);
		printf("File Size:\t%d\n", head.length);

		char Filename[256] = "received/";
		strcat(Filename, head.FILENAME);

		char buffer[head.length];

		FILE *recvFile = fopen(Filename, "wb");

		if (read(conn_fd, &buffer, 1024*1024) == -1){
			perror("Error in receiving data!");
			exit(EXIT_FAILURE);
		}


		fwrite(buffer, 1, sizeof(buffer), recvFile);

		printf("\nClose Connection!\n");
	}

	return 0;

}
