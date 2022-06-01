#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_CHAR_ARRAY_SIZE 100

void errorFound(char* msg) {
    perror(msg);
    exit(0);
}

int main() {
    int socketFileDescriptor, numberBytesReceived;
    struct sockaddr_in server, from;
    char messageToSend[MAX_CHAR_ARRAY_SIZE], messageReceived[MAX_CHAR_ARRAY_SIZE];

    socketFileDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketFileDescriptor == -1) {
        errorFound("[Client] error creating socket\n");
    }

    // set up the server
    int size = sizeof(server);
    bzero(&server, size);
    server.sin_addr.s_addr = inet_addr("192.168.1.12");
    server.sin_family = AF_INET;
    server.sin_port = htons(9002);

    printf("give message\n>");
    fgets(messageToSend, MAX_CHAR_ARRAY_SIZE, stdin);

    int length = sizeof(struct sockaddr_in);
    if (sendto(socketFileDescriptor, messageToSend, strlen(messageToSend)+1, 0, (struct sockaddr*)&server, length) == -1) {
        errorFound("[Client] error sending\n");
    }

    if (recvfrom(socketFileDescriptor, messageReceived, sizeof(messageReceived), 0, (struct sockaddr*)&from, &length) == -1) {
        errorFound("[Client] error receiving\n");
    }

    printf("[Client] Server sent > %s\n", messageReceived);

    return 1;
}
