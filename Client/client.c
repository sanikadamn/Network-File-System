#include "includes.h"

int main()
{
    // start the client
    int client_socket;
    struct sockaddr_in client_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0)
    {
        perror("socket");
        exit(0);
    }
    else
        printf("Socket created successfully.\n");

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(DEFAULT_CLIENT_PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // binding client
    if(bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("bind");
        exit(0);
    }
    else
        printf("Socket bound to address and port.\n");

    if (listen(client_socket, 100) < 0)
    {
        perror("listen");
        exit(0);
    }
    else
        printf("Listening on port %d.\n", DEFAULT_CLIENT_PORT);

}