#include "includes.h"

Server *NS;
tpool_t* thread_pool;

int main()
{
    // start the naming server
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
    server_addr.sin_port = htons(DEFAULT_NS_PORT);
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
        printf("Listening on port %d.\n", DEFAULT_NS_PORT);

    // create thread pool
    thread_pool = tpool_create(NUM_THREADS);

    // connect to storage server
    NS = (Server *)malloc(sizeof(Server));
    NS->server_socket = server_socket;
    NS->server_addr = server_addr;
    
    
    // make a thread that listens for connections from clients
    pthread_t connect_servers;
    pthread_create(&connect_servers, NULL, connectStorageServer, NULL);

    // use thread pool?

    // get client connections
    pthread_t connect_clients;
    pthread_create(&connect_clients, NULL, connectClientToNS, NULL);
}

