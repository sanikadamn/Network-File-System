#include "includes.h"

pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
File files[10000];
filecount = 0;

void *getFileInfo(void *arg)
{
    // get the file paths from the storage server
    // assuming that the storage server sends the files as an array of file paths
    Server *storage = (Server *)arg;

    File **file_paths = (File **)malloc(sizeof(File *) * 1000);
    read(storage->server_socket, file_paths, sizeof(File *) * 1000);
    // lock the file
    pthread_mutex_lock(&file_lock);
    for (int i = 0; i < 1000; i++)
    {
        if (file_paths[i] == NULL)
            break;
        files[filecount] = *file_paths[i];
        filecount++;
    }
    pthread_mutex_unlock(&file_lock);

    return NULL;
}

void *connectStorageServer(void *arg)
{
    while(1)
    {
        // get connections from servers and store them
        int connfd;
        struct sockaddr_in storage_server;
        int len = sizeof(storage_server);
        connfd = accept(NS->server_socket, (struct sockaddr*)&storage_server, &len);
        if (connfd < 0)
        {
            perror("accept");
            // exit(0);
        }
        else
            printf("Connection accepted from %s:%d.\n", inet_ntoa(storage_server.sin_addr), ntohs(storage_server.sin_port));

        // make a thread for the storage server to accept the initial data
        Server *storage = (Server *)malloc(sizeof(Server));
        storage->server_socket = connfd;
        storage->server_addr = storage_server;
        pthread_t get_file_info;
        pthread_create(&get_file_info, NULL, getFileInfo, (void *)storage);
    }
}