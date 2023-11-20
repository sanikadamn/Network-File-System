#include "includes.h"

Server *NS_storage;
Server *NS_client;
tpool_t* thread_pool;

int main()
{
    // start the naming server
    int server_ss_socket, server_client_socket;
    struct sockaddr_in server_ss_addr, server_client_addr;

    server_ss_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_ss_socket < 0)
    {
        perror("socket");
        exit(0);
    }
    else
        printf("Socket for servers created successfully.\n");
    server_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_client_socket < 0)
    {
        perror("socket");
        exit(0);
    }
    else
        printf("Socket for clients created successfully.\n");

    memset(&server_ss_addr, 0, sizeof(server_ss_addr));
    memset(&server_client_addr, 0, sizeof(server_client_addr));


    server_ss_addr.sin_family = AF_INET;
    server_ss_addr.sin_port = htons(DEFAULT_NS_SS_PORT);
    server_ss_addr.sin_addr.s_addr = INADDR_ANY;

    server_client_addr.sin_family = AF_INET;
    server_client_addr.sin_port = htons(DEFAULT_NS_CLIENT_PORT);
    server_client_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_ss_socket, (struct sockaddr*)&server_ss_addr, sizeof(server_ss_addr)) < 0)
    {
        perror("bind");
        exit(0);
    }
    else
        printf("Socket bound to address and port.\n");

    if(bind(server_client_socket, (struct sockaddr*)&server_client_addr, sizeof(server_client_addr)) < 0)
    {
        perror("bind");
        exit(0);
    }
    else
        printf("Socket bound to address and port.\n");

    if(listen(server_ss_socket, 100) < 0)
    {
        perror("listen");
        exit(0);
    }
    else
        printf("Listening for connections from storage servers.\n");
    
    if(listen(server_client_socket, 100) < 0)
    {
        perror("listen");
        exit(0);
    }
    else
        printf("Listening for connections from clients.\n");

    // create thread pool
    thread_pool = tpool_create(NUM_THREADS);

    // connect to storage server
    NS_storage = (Server *)malloc(sizeof(Server));
    NS_storage->server_socket = server_ss_socket;
    NS_storage->server_addr = server_ss_addr;

    // connect to client
    NS_client = (Server *)malloc(sizeof(Server));
    NS_client->server_socket = server_client_socket;
    NS_client->server_addr = server_client_addr;
    
    // make a thread that listens for connections from clients

    // pthread_t connect_servers;
    tpool_work(thread_pool, (void (*)(void *))connectStorageServer, NULL);
    // pthread_create(&connect_servers, NULL, connectStorageServer, NULL);

    // use thread pool?

    // get client connections
    // pthread_t connect_clients;
    tpool_work(thread_pool, (void (*)(void *))connectClientToNS, NULL);
    // pthread_create(&connect_clients, NULL, connectClientToNS, NULL);

    // wait for threads to finish
    tpool_wait(thread_pool);
}

