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

		// Server massacre begins here
		struct InitData init;

		if (recv(conn_fd, &init, sizeof(init), 0) == -1){
			perror("Error in receiving InitData: init!");
			exit(EXIT_FAILURE);
		}

		// I don't want to switch (type). It'll make it look bloated compared to a
		// if - else if - ... - else if - else bridge.

		int type = init.type;

		// TYPE 1: IndexGet ShortList TimeStart TimeFinish
		if (type == 1){
			struct ShortList_Data sld;

			if (recv(conn_fd, &sld, sizeof(sld), 0) == -1){
				perror("Error in receiving ShortList_Data: sld!");
				exit(EXIT_FAILURE);
			}

			long int startTime = sld.startModTime;
			long int endTime = sld.endModTime;

			// TODO: Create a function to handle this and send data back
		}

		// TYPE 2: IndexGet LongList
		else if (type == 2){
			// TODO: Create a function to handle this and send data back
		}

		// TYPE 3: IndexGet RegEx "RegEx"
		else if (type == 3){
			struct RegEx_Data red;

			if (recv(conn_fd, &red, sizeof(red), 0) == -1){
				perror("Error in receiving RegEx_Data: red!");
				exit(EXIT_FAILURE);
			}

			char regex[256];
			strcpy(regex, red.regex);

			// TODO: Create a function to handle this and send data back
		}

		// TYPE 4: FileHash Verify FileName
		else if (type == 4){
			struct FileHashVerify_Data fhvd;

			if (recv(conn_fd, &fhvd, sizeof(fhvd), 0) == -1){
				perror("Error in receiving FileHasVerify_Data: fhvd!");
				exit(EXIT_FAILURE);
			}

			char filename[256];
			strcpy(filename, fhvd.filename);

			// TODO: Create a function to handle this and send data back
		}

		// TYPE 5: FileUpload Filename
		else if (type == 5){
			struct FileUpload_Data fud;

			if (recv(conn_fd, &fud, sizeof(fud), 0) == -1){
				perror("Error in receiving FileUpload_Data: fud!");
				exit(EXIT_FAILURE);
			}

			char filename[300] = "SharedServer/";
			strcat(filename, fud.filename);

			char buffer[fud.filesize];

			FILE *recvFile_Ptr = fopen(filename, "w+b");

			if (read(conn_fd, &buffer, fud.filesize) == -1){
				perror("Error in receiving file: read(conn_fd, &buffer, fud.filesize)!");
				exit(EXIT_FAILURE);
			}

			setvbuf(recvFile_Ptr, buffer, _IOFBF, fud.filesize);
			fwrite(buffer, 1, sizeof(buffer), recvFile_Ptr);

			fclose(recvFile_Ptr);
		}

		// TYPE 6: FileDownload Filename
		else if (type == 6){
			struct FileDownload_Data fdd;

			if (recv(conn_fd, &fdd, sizeof(fdd), 0) == -1){
				perror("Error in receiving FileDownload_Data: fdd!");
				exit(EXIT_FAILURE);
			}

			char filename[300] = "SharedServer/";
			strcat(filename, fdd.filename);

			// TODO: Create a function to handle this and send data back
		}

		else{
			printf("Type:\t%d does not exist. Type: 1 - 6.\n", init.type);
			exit(EXIT_FAILURE);
		}


		printf("\nClose Connection!\n");
	}

	return 0;

}