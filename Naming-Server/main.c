#include "includes.h"

int main()
{
    int server_socket, confd;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
    {
        perror("socket");
        exit(0);
    }
    else
        printf("Socket created successfully.\n");

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // binding server
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(0);
    }
    else
        printf("Socket bound to address and port.\n");

    if (listen(server_socket, 100) < 0)
    {
        perror("listen");
        exit(0);
    }
    else
        printf("Listening on port %d.\n", PORT);

    // connect to storage server


    // make threads
    pthread_t threads[100];
    
}

