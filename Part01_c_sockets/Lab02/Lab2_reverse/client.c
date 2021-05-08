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

int socketFileDescriptor;
struct sockaddr_in server;
char message[MAX_CHAR_ARRAY_SIZE], response[MAX_CHAR_ARRAY_SIZE];

void createSocket() {
	socketFileDescriptor = socket(PF_INET, SOCK_STREAM, 0);
	if (socketFileDescriptor == -1) {
		printf("[Client] something went wrong with socket(), %s\n", strerror(errno));
	}
}

void createServer() {
	memset(&server, 0, sizeof(struct sockaddr_in));

	// set up server
	server.sin_addr.s_addr = inet_addr("192.168.1.12");
	server.sin_family = AF_INET;
	server.sin_port = htons(9002);
}

void connectSocketAndServer() {
	int connectState = connect(socketFileDescriptor, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
	if (connectState == -1) {
		printf("[Client] something went wrong with connect(), %s\n", strerror(errno));
	}
}

void readUserInput() {
	printf("give a message\n> ");
	fgets(message, MAX_CHAR_ARRAY_SIZE, stdin);
}

void sendToServer() {
	//always send sizeof + 1 so u send the NULL char 
	int sendStatus = send(socketFileDescriptor, message, strlen(message) + 1, 0);
	if (sendStatus == -1) {
		printf("[Client] something went wrong with send(), %s\n", strerror(errno));
	}
}

void receiveFromServer() {
	//always send sizeof + 1 so u send the NULL char 
	int recvStatus = recv(socketFileDescriptor, &response, sizeof(response), 0);
	if (recvStatus == -1) {
		printf("[Client] something went wrong with recv(), %s\n", strerror(errno));
	}
}

void printToUser() {
	printf("[Client] The server answerd %s\n", response);
}

int main() {
	
	createSocket();
	createServer();
	connectSocketAndServer();
	readUserInput();
	sendToServer();
	receiveFromServer();
	printToUser();

	return 1;
}