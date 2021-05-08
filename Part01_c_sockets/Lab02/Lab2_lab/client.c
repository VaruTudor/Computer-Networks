#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
    int c = socket(AF_INET, SOCK_STREAM, 0);
    if(c < 0)
    {
        printf("Fail to create socket...\n");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_port = htons(1234);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.1.6");

    if(connect(c, (struct sockaddr*) &server, sizeof(server)) < 0)
    {
        printf("Connection error...\n");
        return 1;
    }

    char buffer[500];
    recv(c, buffer, 250, 0);
    printf("I received: %s .\n", buffer);

    close(c);
    return 0;
}