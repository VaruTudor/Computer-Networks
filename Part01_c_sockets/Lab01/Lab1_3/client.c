#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

int main() {

    // creating the client socket
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);


    // creating the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = inet_addr("192.168.1.6");

    // connecting to the server
    int connection_status;
    connection_status = connect(client_socket, (struct sockaddr*) & server_address, sizeof(server_address));

    if (connection_status < 0) {
        printf("Error at connecting to the server [CLIENT]\n\n");
    }

    char string_to_send[256], string_to_receive[256];

    printf("give your message:\n> ");

    scanf("%[^\n]", string_to_send);

    // Q: do we have to format char data to network data arrangement with htons like for integers?
    send(client_socket, &string_to_send, sizeof(string_to_send), 0);
    recv(client_socket, &string_to_receive, sizeof(string_to_receive), 0);

    printf("server sent the message:\n> %s\n", string_to_receive);

    close(client_socket);

    return 0;
}