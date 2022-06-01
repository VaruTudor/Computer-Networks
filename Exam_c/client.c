#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

#define PORT 7000
#define BACKLOG 5

void handleError(char* message){
    char buffer[100];
    strcat(buffer,"[] error");
    strcat(buffer,message);
    strcat(buffer," -> ");
    strcat(buffer,strerror(errno));
    printf("%s\n",buffer);
    exit(EXIT_FAILURE);
}

fd_set master,readFds;

int main(int argc, char** argv){
	
    struct sockaddr_in client,server;
    int sizeOfAddress = sizeof(struct sockaddr_in);
    
    char question[]="a?", answer[50];
    int sizeOfquestion = sizeof(question);

    // groupLeader - Teacher connection
    int newSocketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (newSocketFileDescriptor == -1) 
        handleError("socket()");

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_addr.s_addr = inet_addr("192.168.1.2");
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_family = AF_INET;

    if (connect(newSocketFileDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) 
        handleError("connect()");

    FD_ZERO(&master);
    FD_ZERO(&readFds);
    FD_SET(0,&master);
    FD_SET(newSocketFileDescriptor, &master);

    while (1) {
        readFds = master;
        char buffer[100];
        
        if(select(newSocketFileDescriptor+1,&readFds,NULL,NULL,NULL) == -1)
            handleError("select() ");
        
        if (FD_ISSET(0,&readFds)){
            int sizeOfResponse = read(0,buffer,sizeof(buffer)-1);
            if(send(newSocketFileDescriptor,buffer,sizeOfResponse,0) < 0)
                printf("error send()");
        }
        
        if (FD_ISSET(newSocketFileDescriptor, &readFds)) {
//            send(newSocketFileDescriptor, question, strlen(question) + 1, 0);
            int sizeOfResponse = read(newSocketFileDescriptor, buffer, sizeof(buffer) - 1);
            if (sizeOfResponse <= 0) {
                printf("Server closed connection\n");
                exit(EXIT_FAILURE);
            }
            write(1, buffer, sizeOfResponse);
        }
    }
    
    //close(socket)
    return 0;
}

