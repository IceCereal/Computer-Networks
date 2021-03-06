// Compile with: gcc -o server -Wall Peer-Server.c -lcrypto -lssl

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
#include <dirent.h>
#include <time.h>
#include <errno.h>

#include "common.h"

#define PORT 3010
#define BUFFER 1024
#define BACKLOG 4

char *ShortList(long int, long int);
char *LongList();
char *FileHash(char *);
char *RegEx(char *);

int main(int argc, char *argv[]){
	int listen_fd, conn_fd;

	struct sockaddr_in servaddr;
	int servaddrlen = sizeof(servaddr);

	pid_t child_pid;

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
		
		child_pid = fork();

		if (child_pid == 0){
			// This is the child process

			// We first close the listener. The parent will handle that
			close(listen_fd);

			// Handle the rest as normal.

			// Server massacre begins here
			struct InitData init;

			if (recv(conn_fd, &init, sizeof(init), 0) == -1){
				perror("Error in receiving InitData: init!");
				exit(EXIT_FAILURE);
			}

			// I don't want to switch (type). It'll make it look bloated compared to a
			// if - else if - ... - else if - else bridge.

			int type;
			char clientname[256];

			type = init.type;
			strcpy(clientname, init.clientName);

			printf("Client:\t%s", clientname);

			// TYPE 1: IndexGet ShortList TimeStart TimeFinish
			if (type == 1){
				struct ShortList_Data sld;

				if (recv(conn_fd, &sld, sizeof(sld), 0) == -1){
					perror("Error in receiving ShortList_Data: sld!");
					exit(EXIT_FAILURE);
				}

				char *time_details_start;
				time_details_start = (char *)malloc(sizeof(char) * 20);
				strcpy(time_details_start, sld.startModTime);
				struct tm tm_start;
				strptime(time_details_start, "%d-%m-%y", &tm_start);
				time_t tStart = mktime(&tm_start);

				char *time_details_end;
				time_details_end = (char *)malloc(sizeof(char) * 20);
				strcpy(time_details_end, sld.endModTime);
				struct tm tm_end;
				strptime(time_details_end, "%d-%m-%y", &tm_end);
				time_t tEnd = mktime(&tm_end);

				struct ShortList_Return_Data slrd;

				strcpy(slrd.output, ShortList(tStart, tEnd));

				if (send(conn_fd, &slrd, sizeof(slrd), 0) == -1){
					perror("\nError in sending ShortList_Return_Data: slrd\n");
					exit(EXIT_FAILURE);
				}
			}

			// TYPE 2: IndexGet LongList
			else if (type == 2){
				struct LongList_Return_Data llrd;

				strcpy(llrd.output, LongList());

				if (send(conn_fd, &llrd, sizeof(llrd), 0) == -1){
					perror("\nError in sending LongList_Return_Data: llrd\n");
					exit(EXIT_FAILURE);
				}
			}

			// TYPE 3: IndexGet RegEx "RegEx"
			else if (type == 3){
				struct RegEx_Data red;

				if (recv(conn_fd, &red, sizeof(red), 0) == -1){
					perror("Error in receiving RegEx_Data: red!");
					exit(EXIT_FAILURE);
				}

				char *foundFiles;
				foundFiles = (char *)malloc(sizeof(char) * 100);

				foundFiles = RegEx(red.regex);

				struct RegEx_Return_Data rerd;

				strcpy(rerd.output, foundFiles);

				if (send(conn_fd, &rerd, sizeof(rerd), 0) == -1){
					perror("\nError in sending RegEx_Return_Data: rerd\n");
					exit(EXIT_FAILURE);
				}

				free(foundFiles);
			}

			// TYPE 4: FileHash Verify FileName
			else if (type == 4){
				struct FileHashVerify_Data fhvd;

				if (recv(conn_fd, &fhvd, sizeof(fhvd), 0) == -1){
					perror("Error in receiving FileHasVerify_Data: fhvd!");
					exit(EXIT_FAILURE);
				}

				char *filename;
				filename = (char *)malloc(sizeof(char) * 256);
				strcpy(filename, "SharedServer/");
				strcat(filename, fhvd.filename);
				
				struct FileHashVerify_Return_Data fhvrd;
				struct stat file_stat;
				stat(filename, &file_stat);

				strftime(fhvrd.lastModTime, 20, "%d-%m-%y", localtime(&(file_stat.st_ctime)));

				char *md5value;
				md5value = (char *)malloc(sizeof(char) * 100);

				md5value = FileHash(filename);
				
				strcpy(fhvrd.md5value, md5value);

				if (send(conn_fd, &fhvrd, sizeof(fhvrd), 0) == -1){
					perror("\nError in sending FileHashVerify_Return_Data: fhvrd\n");
					exit(EXIT_FAILURE);
				}

				free(filename); free(md5value);
			}

			// TYPE 5: FileUpload Filename
			else if (type == 5){
				struct FileUpload_Data fud;

				if (recv(conn_fd, &fud, sizeof(fud), 0) == -1){
					perror("Error in receiving FileUpload_Data: fud!");
					exit(EXIT_FAILURE);
				}

				// Check if client: clientname has a dedicated directory
				char clientdir_name[256] = "Shared-Client-";
				strcat(clientdir_name, clientname);
				DIR *clientdir = opendir(clientdir_name);

				if (clientdir){
					closedir(clientdir);
				} else if (ENOENT == errno){
					if (mkdir(clientdir_name, 0777) == -1){
						perror("\nFailed to make client directory\n");
						exit(EXIT_FAILURE);
					}
				} else{
					perror(strerror(errno));
					exit(EXIT_FAILURE);
				}

				char filename[300];
				strcat(filename, clientdir_name);
				strcat(filename, "/");
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

				struct FileDownload_Return_Data fdrd;

				FILE *filePtr = fopen(filename, "rb");

				if (filePtr == NULL){
					fdrd.status = 0;
					if (send(conn_fd, &fdrd, sizeof(fdrd), 0) == -1){
						perror("\nError in sending FileDownload_Return_Data: fdrd\n");
						exit(EXIT_FAILURE);
					}

					printf("\nClose Connection!\n");
					continue;
				}

				struct stat file_stats;
				stat(filename, &file_stats);

				printf("%ld", file_stats.st_size);

				fdrd.status = 1;
				strcpy(fdrd.filename, fdd.filename);
				fdrd.filesize = file_stats.st_size;

				if (send(conn_fd, &fdrd, sizeof(fdrd), 0) == -1){
					perror("\nError in sending FileDownload_Return_Data: fdrd\n");
					exit(EXIT_FAILURE);
				}

				char buffer[fdrd.filesize];

				fread(buffer, 1, fdrd.filesize, filePtr);

				if (send(conn_fd, buffer, fdrd.filesize, 0) == -1){
					perror("\nError in uploading file to client. send(conn_fd, buffer, fdrd.filesize, 0)\n");
					exit(EXIT_FAILURE);
				}

				fclose(filePtr);
			}

			else{
				printf("Type:\t%d does not exist. Type: 1 - 6.\n", init.type);
				exit(EXIT_FAILURE);
			}


			printf("\nClose Connection!\n");
			exit(EXIT_SUCCESS);

		} else if (child_pid == -1){
			// This failed
			printf("FAILED");
		}
	}

	return 0;

}

char *ShortList(long int StartTime, long int StopTime){
	char *returnString;
	returnString = (char *)malloc(1024*5 * sizeof(char));
	DIR *dir;
	struct dirent *ent;
	struct stat file_stat;
	if ((dir = opendir ("SharedServer/")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if (ent->d_type == DT_REG){
				char tempFile[256] = "SharedServer/";
				strcat(tempFile, ent->d_name);

				stat(tempFile, &file_stat);

				if ((StartTime < file_stat.st_mtime) && (file_stat.st_mtime < StopTime)){
					char size[10];
					char lastMod[20];

					sprintf(size, "%ld", file_stat.st_size);
					sprintf(lastMod, "%ld", file_stat.st_mtime);

					strcat(returnString, ent->d_name);
					strcat(returnString, "\t");
					strcat(returnString, size);
					strcat(returnString, "\t");
					strcat(returnString, lastMod);
					strcat(returnString, "\n");
				}
			}
		}
		closedir (dir);
	}
	return (returnString);
}

char *LongList(){
	char *returnString;
	returnString = (char *)malloc(1024*5 * sizeof(char));
	DIR *dir;
	struct dirent *ent;
	struct stat file_stat;
	if ((dir = opendir ("SharedServer")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if (ent->d_type == DT_REG){
				char tempFile[256] = "SharedServer/";
				strcat(tempFile, ent->d_name);

				stat(tempFile, &file_stat);

				char lastMod[20];
				strftime(lastMod, 20, "%d-%m-%y", localtime(&(file_stat.st_ctime)));

				char size[10];
				sprintf(size, "%ld", file_stat.st_size);

				strcat(returnString, ent->d_name);
				strcat(returnString, "\t"); 
				strcat(returnString, size);
				strcat(returnString, "\t");
				strcat(returnString, lastMod);
				strcat(returnString, "\n");
			}
		}
		closedir (dir);
	}
	strcat(returnString, "\0");
	return returnString;
}

void get_popen(char *exec, char *data){
	FILE *f;

	f = popen(exec, "r");

	fgets(data, 512, f);

	pclose(f);

	return;
}

char *FileHash(char *filename){
	char exec[100] = {0};
	sprintf(exec, "md5sum %s", filename);

	char *data;
	data = (char *)malloc(100 * sizeof(char));
	get_popen(exec, data);

	return data;
}

char *RegEx(char *query){
	char exec[100] = {0};
	sprintf(exec, "find SharedServer/ -maxdepth 1 -name \"%s\" -print", query);

	char *data;
	data = (char *)malloc(100 * sizeof(char));
	get_popen(exec, data);

	return data;
}