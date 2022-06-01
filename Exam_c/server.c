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
    strcat(buffer,"[Server] error");
    strcat(buffer,message);
    strcat(buffer," -> ");
    strcat(buffer,strerror(errno));
    printf("%s\n",buffer);
    exit(EXIT_FAILURE);
}

fd_set master, readFds;
int highestFileDescriptor;
int currentFileDescriptor;
int socketFileDescriptor;
char buf[256];

void sendToAll(char* buffer, int size){
    int j, ret;
    for(j = 0; j <= highestFileDescriptor;j++){
        if(FD_ISSET(j,&master)){
            if(j!=currentFileDescriptor && j!=socketFileDescriptor)
                if(send(j,buffer,size,0) == -1)
                    printf("error\n");
        }
    }
}

struct sockaddr_in getSocketName(int s, int isLocal) {
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    int ret;

    memset(&addr, 0, sizeof(addr));
    if(isLocal){
        if(getsockname(s,(struct sockaddr*)&addr,&addrlen) == -1)
            printf("e\n");
    }
    else{
        if(getpeername(s,(struct sockaddr*)&addr,&addrlen) == -1)
            printf("e\n");
    }
    
    return addr;
}

char * getIPAddress(int s, int local_or_remote) {
    struct sockaddr_in addr;
    addr = getSocketName(s, local_or_remote);
    return inet_ntoa(addr.sin_addr);
}

int getPort(int s, int local_or_remote) {
    struct sockaddr_in addr;
    addr = getSocketName(s, local_or_remote);
    return addr.sin_port;
}

char clientsConnected[1024];

int main(int argc, char** argv){
	
    struct sockaddr_in server, client;
    int clients = 0;
    
    FD_ZERO(&master);
    FD_ZERO(&readFds);
    
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM,0);
    if (socketFileDescriptor == -1)
        handleError("socket() ");
    
    
    int yes = 1;
    if (setsockopt(socketFileDescriptor,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
        handleError("setsockopt() ");
    
    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    
    printf("[Server] started\n");
    
    if (bind(socketFileDescriptor,(struct sockaddr*)&server,sizeof(server)) == -1)
        handleError("bind() ");
    
    if (listen(socketFileDescriptor,BACKLOG) == -1)
        handleError("listen() ");
    
    FD_SET(socketFileDescriptor, &master);
    highestFileDescriptor = socketFileDescriptor;
    
    
    
    while(1){
        readFds = master;
        if(select(highestFileDescriptor+1, &readFds,NULL,NULL,NULL) == -1)
            handleError("select() ");
        
        for(int fileDescriptor = 0; fileDescriptor <= highestFileDescriptor; fileDescriptor++){
            if(FD_ISSET(fileDescriptor,&readFds)){
                currentFileDescriptor = fileDescriptor;
                if(fileDescriptor == socketFileDescriptor){
                    int sizeOfClientAddress = sizeof(client);
                    int newSocketFileDescriptor = accept(socketFileDescriptor, (struct sockaddr*)&client,&sizeOfClientAddress);
                    if (newSocketFileDescriptor == -1)
                        handleError("accept() ");
                    else{
                        // handle new connection if no error in accept()
                        FD_SET(newSocketFileDescriptor,&master);
                        if(newSocketFileDescriptor > highestFileDescriptor)
                            highestFileDescriptor = newSocketFileDescriptor;
                        clients++;
                        
                        printf("new connection from %s on socket %d, we have %d clients, \n",getIPAddress(newSocketFileDescriptor,0),newSocketFileDescriptor,clients);
                        char tempBuffer[1024];
                        sprintf(tempBuffer,"client nr(%d), IP:%s, port:%d\n",clients,getIPAddress(newSocketFileDescriptor,0),getPort(newSocketFileDescriptor,0));
                        strcat(clientsConnected,tempBuffer);
                        send(newSocketFileDescriptor,clientsConnected,strlen(clientsConnected)+1,0);
                    }
                }
                else{
                    char buffer[1024];
                    int sizeOfResponse = recv(fileDescriptor, buffer,sizeof(buffer),0);
                    if (sizeOfResponse < 0){
                        if(sizeOfResponse == 0){
                            printf("[Server] client %d hung up\n",fileDescriptor);
                        }
                        else 
                            perror("recv() ");
                        clients--;
                        close(fileDescriptor);
                        FD_CLR(fileDescriptor,&master);
                    }
                    else{
                        buffer[sizeOfResponse] = 0;
                        if(strncasecmp("QUIT\n",buffer,4) == 0){
                            sprintf(buffer,"disconecting %s",getIPAddress(fileDescriptor,0));
                            send(fileDescriptor,buffer,strlen(buffer+1),0);
                            char tempBuffer[256];
                            int tempSize = sprintf(tempBuffer,"%s - %d disconected",getIPAddress(fileDescriptor,0),fileDescriptor);
                            sendToAll(tempBuffer,tempSize);
                            clients--;
                            close(fileDescriptor);
                            FD_CLR(fileDescriptor,&master);
                        }
                        else{
                            sendToAll(buffer, strlen(buffer));
                        }
                    }
                    
                }
            }
        }
    }
    
    //close(socket)
    return 0;
}
