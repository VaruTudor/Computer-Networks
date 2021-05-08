#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CHAR_ARRAY_SIZE 100

int main() {
	int socketFileDescriptor;
	struct sockaddr_in server;
	char message[MAX_CHAR_ARRAY_SIZE];
	int32_t guess;

	socketFileDescriptor = socket(PF_INET, SOCK_STREAM, 0);
	if (socketFileDescriptor == -1) {
		printf("[Client] something went wrong with socket(), %s\n", strerror(errno));
		return 1;
	}

	// set up server
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_addr.s_addr = inet_addr("192.168.1.2");
	server.sin_family = AF_INET;
	server.sin_port = htons(9002);

	int connectState = connect(socketFileDescriptor, (struct sockaddr*) &server, sizeof(struct sockaddr_in));
	if (connectState == -1) {
		printf("[Client] something went wrong with connect(), %s\n", strerror(errno));
		return 1;
	}

	printf("give a number as a guess\n> ");
	scanf("%hu", &guess);
	guess = htonl(guess);

	//always send sizeof + 1 so u send the NULL char 
	int sendStatus = send(socketFileDescriptor, &guess, sizeof(guess),0);
	if (sendStatus == -1) {
		printf("[Client] something went wrong with connect(), %s\n", strerror(errno));
		return 1;
	}

	recv(socketFileDescriptor, &message, sizeof(message),0);
	printf("[Client] The server answerd %s\n", message);

	return 1;
}