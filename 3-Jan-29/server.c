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

	uint32_t raw_data[150];
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
		int N = (raw_data[0]);

		// Parity check arrays sent after N
		int **parity_check = (int **)malloc(2 * sizeof(int *));
		parity_check[0] = (int *)malloc(N * sizeof(int)); // Row
		parity_check[1] = (int *)malloc(N * sizeof(int)); // Column

		// array sent after parity check arrays.
		int **array = (int **)malloc(N * sizeof(int *));
		for (i = 0; i < N; ++i)
			array[i] = (int *)malloc(N * sizeof(int));
		
		for (j = 0; j < N; ++j){
			parity_check[0][j] = (raw_data[1 + j]);
			parity_check[1][j] = (raw_data[1 + N + j]);
		}

		for (i = 0; i < N; ++i)
			for (j = 0; j < N; ++j)
				array[i][j] = (raw_data[(2*N)+1 + (i*N + j)]);

		for (i = 0; i < N; ++i){
			for (j = 0; j < N; ++j)
				printf("%d ", array[i][j]);
			printf("\n");
		}

		int **parity_check_server = (int **)malloc(2 * sizeof(int *));
		parity_check_server[0] = (int *)malloc(N * sizeof(int)); // Row
		parity_check_server[1] = (int *)malloc(N * sizeof(int)); // Column

		// Parity total in server
		for (i = 0; i < N; ++i){
			int sum_row = 0, sum_column = 0;
			for (j = 0; j < N; ++j){
				sum_row += array[i][j];
				sum_column += array[j][i];
			}
			parity_check_server[0][i] = sum_row;
			parity_check_server[1][i] = sum_column;
		}

		// Figure out which row and column have been altered
		int alter_row, alter_col;
		for (i = 0; i < N; ++i){
			if (parity_check_server[0][i] != parity_check[0][i])
				alter_row = i;
			if (parity_check_server[1][i] != parity_check[1][i])
				alter_col = i;
		}

		printf("\nServer detects the following have been altered:\nRow:\t%d\nCol:\t%d\n\n", alter_row, alter_col);

		// clean up
		for (i = 0; i < 2; ++i){
			free(parity_check[i]);
			free(parity_check_server[i]);
		}
		free(parity_check);
		free(parity_check_server);
		for (i = 0; i < N; ++i)
			free(array[i]);
		free(array);

		printf("Close Connection!\n\n");
	}

	return 0;

}