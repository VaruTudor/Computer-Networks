#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CHAR_COUNT 100

int main() {

	// create the client socket
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	// handle possible error
	if (clientSocket == -1) {
		printf("[Client] Error at creating the socket\n");
		return 1;
	}

	struct sockaddr_in server;
	server.sin_port = htons(9002);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("192.168.1.6");

	// connect to the server
	int connectionToServerStatus = connect(clientSocket, (struct sockaddr*)&server, sizeof(server));

	// reading and sending the string
	char stringToSend[MAX_CHAR_COUNT];
	printf("Give a string of max %d chars\n", MAX_CHAR_COUNT);
	fgets(stringToSend, MAX_CHAR_COUNT, stdin);

	int sendingStringStatus = send(clientSocket, stringToSend, strlen(stringToSend) + 1, 0);
	if (sendingStringStatus == -1) {
		printf("[Client] Error at sending data to server");
		return 1;
	}

	char serverResponse[MAX_CHAR_COUNT];
	int receiveStringStatus = recv(clientSocket, &serverResponse, sizeof(serverResponse), MSG_WAITALL);
	if (receiveStringStatus == -1) {
		printf("[Client] Error at receiving data from server");
		return 1;
	}


	printf("Server returned %s \n", serverResponse);

	close(clientSocket);

	return 1;
}