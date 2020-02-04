#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 3010

int main(int argc, char *argv[]){
	/*
		Usage: ./a.out N
			N (int): The size of the NxN matrix desired by the user (max: 10)
	*/
	if (argc != 3){
		printf("Argument count not 2.\nUsage: ./a.out N Seed\n");
		exit(EXIT_FAILURE);
	}

	int N = atoi(argv[1]);
	int seed = atoi(argv[2]);
	int i, j;
	uint32_t array[150];

	if (N > 10){
		printf("N cannot be greater than 10.\n");
		exit(EXIT_FAILURE);
	} if (N == 0){
		printf("N cannot be 0.\n");
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

	// Generate matrix

	array[0] = (N);

	for (i = 0; i < N; ++i){
		for (j = 0; j < N; ++j){
			srand(i + j + (seed * j) + (seed * i * j));
			array[(2*N)+1 + (i*N + j)] = (rand() % 2);
		}
	}

	// Generate parity-store
	for (i = 0; i < N; ++i){
		int sum_column = 0, sum_row = 0;
		for (j = 0; j < N; ++j){
			sum_row += array[(2*N)+1 + (i*N +j)];
			sum_column += array[(2*N)+1 + (j*N +i)];
		}
		array[i+1] = (sum_row);
		array[N+i+1] = (sum_column);
	}

	for (i = 0; i < N; ++i){
		for (j = 0; j < N; ++j){
			printf("%d ", (array[(2*N)+1 + (i*N + j)]));
		}
		printf("\n");
	}

	// Begin mess up
	int row_mess, col_mess;
	srand(seed);
	row_mess = rand() % N;
	srand(seed * seed);
	col_mess = rand() % N;

	printf("\nAttempting to mess up:\t%d, %d\n", row_mess, col_mess);

	array[(2*N)+1 + (row_mess*N + col_mess)] = (!array[(2*N)+1 + (row_mess*N + col_mess)]);

	// for (i = 0; i < N; ++i){
	// 	for (j = 0; j < N; ++j){
	// 		printf("%d ", array[(2*N)+1 + (i*N + j)]);
	// 	}
	// 	printf("\n");
	// }

	if (send(sock_fd, array, sizeof(array), 0) == -1){
		perror("Sending data via socket");
		exit(EXIT_FAILURE);
	}


	return 0;
}