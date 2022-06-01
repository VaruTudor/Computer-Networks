#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 9002   // port we're listening on

fd_set allClientsFileDescriptors;   // clients file descriptor list
fd_set temporaryFileDescriptors;    // temp file descriptor list for select()
struct sockaddr_in serverAddress;   // server address
struct sockaddr_in remoteAddress;   // client address
int highestFileDescriptor;        // maximum file descriptor number
int listenerFileDescriptor;     // listening socket descriptor
int acceptedFileDescriptor;        // newly accept()ed socket descriptor
char buffer[256], temporaryBuffer[256];    // bufferfer for client data
int nbytes, ret;
int yes = 1;        // for setsockopt() SO_REUSEADDR, below
int addrlen;
int i, j, crt, int_port, client_count = 0;

/*
* isLocal (type int) 1 for local, 0 for remote
*/
struct sockaddr_in getSocketName(int socketFileDescriptor, int isLocal) {
    struct sockaddr_in newAddress;
    int sizeOfAddress = sizeof(newAddress);
    memset(&newAddress, 0, sizeof(newAddress));

    if (isLocal) {
        if (getsockname(socketFileDescriptor, (struct sockaddr*)&newAddress, (socklen_t*)&sizeOfAddress) == -1) {
            printf("[Server] error with getsockname() - %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else {
        if (getpeername(socketFileDescriptor, (struct sockaddr*)&newAddress, (socklen_t*)&sizeOfAddress) == -1) {
            printf("[Server] error with getsockname() - %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    return newAddress;
}

char* getIPAddress(int socketFileDescriptor, int isLocal) {
    struct sockaddr_in addr;
    addr = getSocketName(socketFileDescriptor, isLocal);
    return inet_ntoa(addr.sin_addr);
}

int getPort(int socketFileDescriptor, int isLocal) {
    struct sockaddr_in addr;
    addr = getSocketName(socketFileDescriptor, isLocal);
    return addr.sin_port;
}

// send to everyone
void sendToALL(char* buffer, int nbytes) {
    int currentFileDescriptor;
    for (currentFileDescriptor = 0; currentFileDescriptor <= highestFileDescriptor; currentFileDescriptor++) {
        if (FD_ISSET(currentFileDescriptor, &allClientsFileDescriptors))
            // except the listenerFileDescriptor and ourselves
            if (currentFileDescriptor != listenerFileDescriptor && currentFileDescriptor != crt)
                if (send(currentFileDescriptor, buffer, nbytes, 0) == -1)
                    perror("send");
    }
    return;
}


int main(int argc, char** argv)
{

    FD_ZERO(&allClientsFileDescriptors);    // clear the allClientsFileDescriptors and temp sets
    FD_ZERO(&temporaryFileDescriptors);

    // get the listenerFileDescriptor
    if ((listenerFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // lose the pesky "address already in use" error message
    if (setsockopt(listenerFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt:");
        exit(EXIT_FAILURE);
    }

    // bind
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);
    //    memset(&(serverAddress.sin_zero), '\0', 8);
    if (bind(listenerFileDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("bind:");
        exit(EXIT_FAILURE);
    }
    // listen
    if (listen(listenerFileDescriptor, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // add the listenerFileDescriptor to the allClientsFileDescriptors set
    FD_SET(listenerFileDescriptor, &allClientsFileDescriptors);
    // keep track of the biggest file descriptor
    highestFileDescriptor = listenerFileDescriptor; // so far, it's this one
    // main loop
    for (;;) {
        temporaryFileDescriptors = allClientsFileDescriptors; // copy it
        if (select(highestFileDescriptor + 1, &temporaryFileDescriptors, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        // run through the existing connections looking for data to read
        for (i = 0; i <= highestFileDescriptor; i++) {
            if (FD_ISSET(i, &temporaryFileDescriptors)) { // we got one!!
                crt = i;
                if (i == listenerFileDescriptor) {
                    // handle new connections
                    addrlen = sizeof(remoteAddress);
                    if ((acceptedFileDescriptor = accept(listenerFileDescriptor, (struct sockaddr*)&remoteAddress, (socklen_t*)&addrlen)) == -1) {
                        perror("accept");
                    }
                    else {
                        FD_SET(acceptedFileDescriptor, &allClientsFileDescriptors); // add to allClientsFileDescriptors set
                        if (acceptedFileDescriptor > highestFileDescriptor) {    // keep track of the maximum
                            highestFileDescriptor = acceptedFileDescriptor;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n", getIPAddress(acceptedFileDescriptor, 0), acceptedFileDescriptor);

                        client_count++;
                        sprintf(buffer, "Hi-you are client :[%d] (%s:%d) connected to server %s\nThere are %d clients connected\n",
                            acceptedFileDescriptor, getIPAddress(acceptedFileDescriptor, 0), getPort(acceptedFileDescriptor, 0),
                            getIPAddress(listenerFileDescriptor, 1), client_count);
                        send(acceptedFileDescriptor, buffer, strlen(buffer) + 1, 0);
                    }
                }
                else {
                    // handle data from a client
                    nbytes = recv(i, buffer, sizeof(buffer), 0);
                    if (nbytes == 0) {
                        // connection closed
                        printf("<selectserver>: client %d forcibly hung up\n", i);
                        client_count--;
                        close(i); // bye!
                        FD_CLR(i, &allClientsFileDescriptors); // remove from allClientsFileDescriptors set
                    }
                    if (nbytes == -1) {
                        // got error or connection closed by client
                        perror("recv");
                        client_count--;
                        close(i); // bye!
                        FD_CLR(i, &allClientsFileDescriptors); // remove from allClientsFileDescriptors set
                    }
                    if (nbytes > 0) {
                        buffer[nbytes] = 0;
                        // we got some data from a client
                        nbytes = sprintf(temporaryBuffer, "<%s - [%d]> %s", getIPAddress(crt, 0), crt, buffer);
                        sendToALL(temporaryBuffer, nbytes);
                    }
                }
            }
        }
    }

    return 0;
}