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
		Usage: ./a.out seed "string"
			string (char *): A character array of size 10
	*/
	if (argc != 3 ){
		printf("Argument count should be 3.\nUsage: ./a.out seed \"string\" ");
		exit(EXIT_FAILURE);
	}

	int seed = atoi(argv[1]);
	int i;

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
	uint32_t bitsize = sizeof(uint8_t) * 8;
	uint8_t length = (strlen(argv[2]) > 10) ? 10 : strlen(argv[2]);

	uint8_t metadata[bitsize + length];
	for (i = 0; i < bitsize+length; ++i)
		metadata[i] = 0;


	uint8_t string[length];
	for (i = 0; i < length; ++i)
		string[i] = argv[2][i];

	for (int strNum = 0; strNum < length; ++strNum){
		int rowBitSum = 0;
		for (i = bitsize - 1; i >=0; --i){
			int mask = 1 << i;
			if (string[strNum] & mask){
				metadata[bitsize - 1 - i]++;
				rowBitSum++;
			}
		}
		metadata[bitsize + strNum] = rowBitSum;
	}

	/* VERBOSE */
	// printf("Original String:\t");
	// for (i = 0; i < length; ++i)
	// 	printf("%d ", string[i]);

	/* VERBOSE */
	// printf("\n");
	// for (i = 0; i < 8; ++i){
	// 	printf("%d ", metadata[i]);
	// } printf("\n");
	// for (i = 8; i < length+8; ++i){
	// 	printf("%d ", metadata[i]);
	// }
	
	srand(seed);
	uint8_t strNum_MessUp = rand() % length;
	srand(seed * seed);
	uint8_t bitNum_MessUp = rand() % bitsize;
	// printf("\n\nNumber: %d\tBit:%d", string[strNum_MessUp], bitNum_MessUp);

	uint8_t mask_MessUp = 1 << (bitNum_MessUp);
	uint8_t newNum = 0;

	for (i = bitsize; i >= bitNum_MessUp+1; --i){
		int mask = 1 << i;
		if (string[strNum_MessUp] & mask){
			newNum = newNum | mask;
		}
	}
	if (!(string[strNum_MessUp] & mask_MessUp)){
		newNum = newNum | mask_MessUp;
	}
	for (i = bitNum_MessUp - 1; i >= 0; --i){
		int mask = 1 << i;
		if (string[strNum_MessUp] & mask){
			newNum = newNum | mask;
		}
	}

	/* VERBOSE */
	// printf("\nNew Number: %d", newNum);
	string[strNum_MessUp] = newNum;

	/* VERBOSE */
	// printf("\nAltered String:\t");
	// for (i = 0; i < length; ++i)
	// 	printf("%d ", string[i]);

	uint8_t array[1 + bitsize+length + length * sizeof(uint8_t)];

	array[0] = length;
	for (i = 0; i < bitsize + length; ++i)
		array[i+1] = metadata[i];

	for (i = 0; i < length; ++i)
		array[1 + bitsize+length + i] = string[i];

	if (send(sock_fd, array, sizeof(array), 0) == -1){
		perror("Sending data via socket");
		exit(EXIT_FAILURE);
	}

	return 0;
}