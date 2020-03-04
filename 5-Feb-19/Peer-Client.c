// Compile with: gcc -o client -Wall Peer-Client.

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

#include "common.h"

#define PORT 3010
#define BUFFER 1024

char *get_line();
char **parse_line(char *);
int execute_input(char **, char *);

int connection(int type, char **args);

int main(int argc, char *argv[]){
	/*
		Usage: ./a.out
	*/
	if (argc != 1){
		printf("Argument count should be 1.\nUsage: ./a.out\n");
		exit(EXIT_FAILURE);
	}

	int statusFlag = 1;

	do{
		char *inputLine;
		char **args;
		printf("\n> ");
		inputLine = get_line();
		args = parse_line(inputLine);
		statusFlag = execute_input(args, inputLine);
	
	} while(statusFlag);


	return 0;
}

char *get_line(){
	char *line = NULL;
	size_t buffersize = 0;
	getline(&line, &buffersize, stdin);

	return line;
}

char **parse_line(char *line){
	int totalArgs = 0;
	int max_argLen = 0;

	char *command = (char *)malloc(strlen(line) * sizeof(char));
	char *lineTemp = (char *)malloc(strlen(line) * sizeof(char));
	strcpy(command, line);
	strcpy(lineTemp, line);

	// Initial loop for figuring out the number of arguments and
	// max_length(argument)
	char* tok = strtok(lineTemp, " \t\n");
	while(tok != NULL){
		totalArgs++;

		int tempLen = strlen(tok);

		if (tempLen > max_argLen)
			max_argLen = tempLen;

		tok = strtok(NULL, " \t\n");
	}

	// Allocate space for the tokens
	char **token = (char **)malloc((totalArgs + 1) * sizeof(char *));
	for (int i = 0; i < totalArgs + 1; ++i)
		token[i] = (char *)malloc((max_argLen) * sizeof(char));

	int argv_counter = 0;
	tok = strtok(command, " \t\n");
	while (tok != NULL){
		strcpy(token[argv_counter++], tok);
		tok = strtok(NULL, " \t\n");
	}

	token[argv_counter] = NULL;

	free(command);
	free(lineTemp);

	return token;
}

int IndexGet(char **);
int FileHash(char **);
int FileDownload(char **);
int FileUpload(char **);

char *builtin_str[] = {
	"IndexGet",
	"FileHash",
	"FileDownload",
	"FileUpload"
};

int (*builtin_func[]) (char **) = {
	&IndexGet,
	&FileHash,
	&FileDownload,
	&FileUpload
};

int num_builtin(){
	return (sizeof(builtin_str) / sizeof(char *));
}

int execute_input(char **args, char *line){
	if (args[0] == NULL)
		return 1;

	int number_builtin = num_builtin();

	for (int i = 0; i < number_builtin; ++i){
		if (strcmp(args[0], builtin_str[i]) == 0)
			return (*builtin_func[i])(args);
	}

	return 0;
}

int IndexGet(char **args){
	if (args[1] == NULL){
		printf("\nUsage:\tIndexGet ShortList|LongList|RegEx\n");
		return 1;
	} else if (strcmp(args[1], "ShortList") == 0){
		if ((args[2] != NULL) && (args[3] != NULL)){
			// TODO: Validate args[2] and args[3], somehow
			return connection(1, args);
		} else{
			printf("\nUsage:\tIndexGet ShortList StartingTimeStamp EndingTimeStamp\n");
			return 1;
		}
	} else if (strcmp(args[1], "LongList") == 0){
		return connection(2, args);
	}else if (strcmp(args[1], "RegEx") == 0){
		if (args[2] != NULL)
			return connection(3, args);
		printf("\nUsage:\tIndexGet RegEx <\"RegEx\">");
		return 1;
	} else{
		printf("\nUsage:\tIndexGet ShortList|LongList|RegEx\n");
		return 1;
	}
}

int FileHash(char **args){
	if (args[1] == NULL){
		printf("\nUsage:\tFileHash Verify <FileName>\n");
		return 1;
	} else if (strcmp(args[1], "Verify") == 0){
		if (args[2] != NULL){
			return connection(4, args);
		} else{
			printf("\nUsage:\tFileHash Verify <FileName>\n");
			return 1;
		}
	} else{
		printf("\nUsage:\tFileHash Verify <FileName>\n");
		return 1;
	}
}

int FileUpload(char **args){
	if (args[1] == NULL){
		printf("\nUsage:\tFileUpload <FileName>\n");
		return 1;
	} else{
		int flag_fileFound = 0;
		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir (".")) != NULL) {
			while ((ent = readdir (dir)) != NULL) {
				if (strcmp(args[1], ent->d_name) == 0)
					flag_fileFound = 1;
			}
			closedir (dir);
		}

		if (!flag_fileFound){
			printf("\nFile doesn't exist!\n");
			return 1;
		}

		return connection(5, args);
	}
}

int FileDownload(char **args){
	if (args[1] == NULL){
		printf("\nUsage:\tFileDownload <FileName>\n");
		return 1;
	} else{
		return connection(6, args);
	}
}

int connection(int type, char **args){
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

	// Let the massacre begin
	// NOTE TO SELF: DOCUMENT **EVERYTHING**
	struct InitData init;
	
	init.type = type;

	// First we send type (through struct InitData)
	if (send(sock_fd, &init, sizeof(init), 0) == -1){
		perror("\nError in sending InitData: init\n");
		return 1;
	}

	// I don't want to switch (type). It'll make it look bloated compared to a
	// if - else if - ... - else if - else bridge.

	// TYPE 1: IndexGet ShortList TimeStart TimeFinish
	if (type == 1){
		struct ShortList_Data sld;
		strcpy(sld.startModTime, args[2]);
		strcpy(sld.endModTime, args[3]);

		if (send(sock_fd, &sld, sizeof(sld), 0) == -1){
			perror("\nError in sending ShortList_Data: sld\n");
			return 1;
		}

		struct ShortList_Return_Data slrd;

		if (recv(sock_fd, &slrd, sizeof(slrd), 0) == -1){
			perror("Error in receiving ShortList_Return_Data: slrd!");
			return 1;
		}

		printf("\n%s", slrd.output);

		return 1;
	}

	// TYPE 2: IndexGet LongList
	else if (type == 2){
		struct LongList_Return_Data llrd;

		if (recv(sock_fd, &llrd, sizeof(llrd), 0) == -1){
			perror("Error in receiving LongList_Return_Data: llrd!");
			return 1;
		}

		printf("%s", llrd.output);

		return 1;
	}

	// TYPE 3: IndexGet RegEx "RegEx"
	else if (type == 3){
		struct RegEx_Data red;
		strcpy(red.regex, args[2]);

		if (send(sock_fd, &red, sizeof(red), 0) == -1){
			perror("\nError in sending RegEx_Data: red\n");
			return 1;
		}

		// TODO: RECV DATA & OUTPUT
		return 1;
	}

	// TYPE 4: FileHash Verify FileName
	else if (type == 4){
		struct FileHashVerify_Data fhvd;
		strcpy(fhvd.filename, args[2]);

		if (send(sock_fd, &fhvd, sizeof(fhvd), 0) == -1){
			perror("\nError in sending FileHashVerify_Data: fhvd\n");
			return 1;
		}

		struct FileHashVerify_Return_Data fhvrd;
		
		if (recv(sock_fd, &fhvrd, sizeof(fhvrd), 0) == -1){
			perror("Error in receiving FileHashVerify_Return_Data: fhvrd!");
			return 1;
		}

		printf("\n%s\t%s\t%s\n", fhvd.filename, fhvrd.md5value, fhvrd.lastModTime);

		return 1;
	}

	// TYPE 5: FileUpload Filename
	else if (type == 5){
		struct FileUpload_Data fud;
		struct stat file_stats;

		strcpy(fud.filename, args[1]);

		stat(fud.filename, &file_stats);

		fud.filesize = file_stats.st_size;

		if (send(sock_fd, &fud, sizeof(fud), 0) == -1){
			perror("\nError in sending FileUpload_Data: fud\n");
			return 1;
		}

		FILE *filePtr = fopen(fud.filename, "rb");

		char buffer[fud.filesize];

		fread(buffer, 1, fud.filesize, filePtr);

		if (send(sock_fd, buffer, fud.filesize, 0) == -1){
			perror("\nError in uploading file. send(socket, buffer, filesize, 0)\n");
			return 1;
		}

		fclose(filePtr);

		return 1;
	}

	// TYPE 6: FileDownload Filename
	else if (type == 6){
		struct FileDownload_Data fdd;
		strcpy(fdd.filename, args[1]);

		if (send(sock_fd, &fdd, sizeof(fdd), 0) == -1){
			perror("\nError in sending FileDownload_Data: fdd\n");
			return 1;
		}

		struct FileDownload_Return_Data fdrd;
		fdrd.status = 0;
		fdrd.filesize = 0;
		strcpy(fdrd.filename, "");

		if (recv(sock_fd, &fdrd, sizeof(fdrd), 0) == -1){
			perror("Error in receiving FileDownload_Return_Data: fdrd!");
			return 1;
		}

		if (!fdrd.status){
			printf("\nFile doesn't exist on server!\n");
			return 1;
		}

		char filename[300] = "SharedClient/";
		strcat(filename, fdd.filename);

		char buffer[fdrd.filesize];

		FILE *recvFile_Ptr = fopen(filename, "wb");

		if (read(sock_fd, &buffer, fdrd.filesize) == -1){
			perror("Error in receiving file: read(sock_fd, &buffer, fdrd.filesize)!");
			return 1;
		}

		setvbuf(recvFile_Ptr, buffer, _IOFBF, fdrd.filesize);
		fwrite(buffer, 1, sizeof(buffer), recvFile_Ptr);

		fclose(recvFile_Ptr);

		return 1;
	}

	// TYPE ...? There should be no more types
	else{
		printf("\nType:\t%d is undefined. Type can only be between 1-6.\n", type);
		return 1;
	}
}