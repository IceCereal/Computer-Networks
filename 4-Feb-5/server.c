#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 3010
#define BUFFER 1024
#define BACKLOG 4

int main(int argc, char *argv[]){
	int listen_fd, conn_fd;

	int bitsize = sizeof(uint8_t) * 8;
	uint8_t raw_data[1 + bitsize+10 + 10 * sizeof(uint8_t)];

	int i, j;

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

		if (recv(conn_fd, &raw_data, BUFFER, 0) == -1){
			perror("Error in receiving data!");
			exit(EXIT_FAILURE);
		}

		// Size of array
		uint8_t length = raw_data[0];

		/* VERBOSE */
		// for (i = 0; i < 1 + length + bitsize + length * sizeof(uint8_t); ++i)
		// 	printf("%d ", raw_data[i]);

		/* VERBOSE */
		// printf("\nLength:\t%d\n", length);

		// Parity check arrays sent after length
		uint8_t **parity_check = (uint8_t **)malloc(2 * sizeof(uint8_t *));
		parity_check[0] = (uint8_t *)malloc(bitsize * sizeof(uint8_t)); // Column
		parity_check[1] = (uint8_t *)malloc(length * sizeof(uint8_t)); // Row

		// array sent after parity check arrays.
		uint8_t *array = (uint8_t *)malloc(length * sizeof(uint8_t));

		for (j = 0; j < bitsize; ++j)
			parity_check[0][j] = raw_data[1 + j];
		for (j = 0; j < length; ++j)
			parity_check[1][j] = raw_data[1 + bitsize + j];

		/* VERBOSE */
		// printf("\n");
		// for (j = 0; j < bitsize; ++j)
		// 	printf("%d ", parity_check[0][j]);
		// printf("\n");
		// for (j = 0; j < length; ++j)
		// 	printf("%d ", parity_check[1][j]);

		for (i = 0; i < length; ++i)
			array[i] = (raw_data[1 + length+bitsize + i]);

		/* VERBOSE */
		// printf("\n");
		// for (i = 0; i < length; ++i)
		// 	printf("%d ", array[i]);

		uint8_t **parity_check_server = (uint8_t **)malloc(2 * sizeof(uint8_t *));
		parity_check_server[0] = (uint8_t *)malloc(bitsize * sizeof(uint8_t)); // Column
		parity_check_server[1] = (uint8_t *)malloc(length * sizeof(uint8_t)); // Row

		for (i = 0; i < bitsize; ++i) parity_check_server[0][i] = 0;
		for (i = 0; i < length; ++i) parity_check_server[1][i] = 0;

		// Parity total in server
		for (int strNum = 0; strNum < length; ++strNum){
			int rowBitSum = 0;
			for (i = bitsize - 1; i >= 0; --i){
				int mask = 1 << i;
				if (array[strNum] & mask){
					parity_check_server[0][bitsize - 1 - i]++;
					rowBitSum++;
				}
			}
			parity_check_server[1][strNum] = rowBitSum;
		}

		char string[length];
		for (i = 0; i < length; ++i)
			string[i] = array[i];
		printf("Infected String:\t%s", string);
		
		/* VERBOSE */
		// printf("\n\nServer Add:\n");
		// for (j = 0; j < bitsize; ++j)
		// 	printf("%d ", parity_check_server[0][j]);
		// printf("\n");
		// for (j = 0; j < length; ++j)
		// 	printf("%d ", parity_check_server[1][j]);

		// // Figure out which row and column have been altered
		int alter_row, alter_col;
		for (i = 0; i < bitsize; ++i)
			if (parity_check_server[0][i] != parity_check[0][i])
				alter_col = i;
		for (i = 0; i < length; ++i)
			if (parity_check_server[1][i] != parity_check[1][i])
				alter_row = i;
		

		/* VERBOSE */
		// printf("\nServer detects the following have been altered:\nCol:\t%d\nRow:\t%d\n\n", alter_col, alter_row);

		int convBack_StrNum = alter_row;
		int convBack_BitNum = bitsize - 1 - alter_col;

		int convBack_Mask = 1 << convBack_BitNum;
		int newNum = 0;

		for (i = bitsize; i >= convBack_BitNum+1; --i){
			int mask = 1 << i;
			if (array[convBack_StrNum] & mask){
				newNum = newNum | mask;
			}
		}
		if (!(array[convBack_StrNum] & convBack_Mask)){
			newNum = newNum | convBack_Mask;
		}
		for (i = convBack_BitNum - 1; i >= 0; --i){
			int mask = 1 << i;
			if (array[convBack_StrNum] & mask){
				newNum = newNum | mask;
			}
		}

		array[convBack_StrNum] = newNum;

		for (i = 0; i < length; ++i)
			string[i] = array[i];
		printf("\nDisinfected String:\t%s", string);

		/* VERBOSE */
		// printf("\n\n%d", newNum);

		// clean up
		for (i = 0; i < 2; ++i){
			free(parity_check[i]);
			free(parity_check_server[i]);
		}
		free(parity_check);
		free(parity_check_server);
		free(array);

		printf("\nClose Connection!\n\n");
	}

	return 0;

}
