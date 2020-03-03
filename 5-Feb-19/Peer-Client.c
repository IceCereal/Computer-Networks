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
		printf("\n>\t");
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

int execute_input(char **args, char *line){
	if (args[0] == NULL)
		return 1;

	int num_builtin = icsh_num_builtin();

	for (int i = 0; i < num_builtin; ++i){
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
			return connection(1, args);
		}
	} else if (strcmp(args[1], "LongList") == 0){
		return connection(2, args);
	}else if (strcmp(args[1], "RegEx") == 0){
		return connection(3, args);
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
			int flag_fileFound = 0;
			DIR *dir;
			struct dirent *ent;
			if ((dir = opendir (".")) != NULL) {
				while ((ent = readdir (dir)) != NULL) {
					if (strcmp(args[2], ent->d_name) == 0)
						flag_fileFound = 1;
				}
				closedir (dir);
			}

			if (!flag_fileFound){
				printf("\nFile doesn't exist!\n");
				return 1;
			}

			return connection(4, args);
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