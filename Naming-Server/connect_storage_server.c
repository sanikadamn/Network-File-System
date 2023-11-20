#include "includes.h"

pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
File *files[10000];

/**
* IP: 
* NPORT:
* CPORT:
* NUMFILES:
* FILENAME:
* FILESIZE:
**/

int filecount = 0;

void *getFileInfo(void *arg)
{
    // get the file paths from the storage server
    // assuming that the storage server sends the files as an array of file paths
    Server *storage = (Server *)arg;

    // read one file packet
    buf_t file_packet;
    buf_malloc(&file_packet, sizeof(str_t), 2048);

    while(1)
    {
        int bytes_read = recv(storage->server_socket, file_packet.data, sizeof(buf_t), MSG_PEEK);
        if (bytes_read < 0)
        {
            perror("read");
            break;
        }
        if (bytes_read == 0)
        {
            printf("Done reading\n");
            break;
        }
        // convert packet to struct
        buf_t *ip = read_str(storage->server_socket, "IP:");
        buf_t *nport = read_str(storage->server_socket, "NPORT:");
        buf_t *cport = read_str(storage->server_socket, "CPORT:");
        i32 numfiles = read_i32(storage->server_socket, "NUMFILES:");
        buf_t *filename = read_str(storage->server_socket, "FILENAME:");
        i64 filesize = read_i64(storage->server_socket, "FILESIZE:");

        // store this file in the file array
        pthread_mutex_lock(&file_lock);
        strcpy(files[filecount]->ss_ip, CAST(char, ip->data));
        strcpy(files[filecount]->ns_port, CAST(char, nport->data));
        strcpy(files[filecount]->client_port, CAST(char, cport->data));
        files[filecount]->num_files = numfiles;
        strcpy(files[filecount]->filename, CAST(char, filename->data));
        files[filecount]->filesize = filesize;
        files[filecount]->storageserver = storage->server_addr;
        files[filecount]->storageserver_socket = storage->server_socket;
        filecount++;
        pthread_mutex_unlock(&file_lock);
    }

    return NULL;
}

void *connectStorageServer(void *arg)
{
    // allocate memory for each file
    for (int i = 0; i < 10000; i++)
    {
        files[i] = (File *)malloc(sizeof(File));
    }
    while(1)
    {
        // get connections from servers and store them
        int connfd;
        struct sockaddr_in storage_server;
        socklen_t len = sizeof(storage_server);
        connfd = accept(NS_storage->server_socket, (struct sockaddr*)&storage_server, &len);
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
        tpool_work(thread_pool, (void (*)(void *))getFileInfo, (void *)storage);
        // pthread_create(&get_file_info, NULL, getFileInfo, (void *)storage);
    }
}