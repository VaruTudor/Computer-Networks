#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

void createClientSocket();

int main() {


	struct sockaddr_in server;

	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
		printf("Error creating the server");
		return 1;
	}

	memset(&server, 0, sizeof(server));

	server.sin_port = htons(9002);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("192.168.1.6");

	int connectionStatus = connect(clientSocket, (struct sockaddr*)&server, sizeof(server));
	if (connectionStatus < 0) {
		printf("Error connecting to the server");
		return 1;
	}

	char charArray1[256], charArray2[256];
	printf("Give arr 1\n");
	fgets(charArray1, sizeof(charArray1), stdin);
	printf("Give arr 2\n");
	fgets(charArray2, sizeof(charArray2), stdin);

	send(clientSocket, &charArray1, sizeof(charArray1), 0);
	send(clientSocket, &charArray2, sizeof(charArray2), 0);

	char charArrayReceived[256];
	recv(clientSocket, &charArrayReceived, sizeof(charArrayReceived), 0);

	printf("the server sent\n%s\n", charArrayReceived);
	close(clientSocket);

	return 1;
}

