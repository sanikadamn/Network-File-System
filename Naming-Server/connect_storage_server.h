#ifndef CONNECTSERVER_H
#define CONNECTSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../common/thread_pool.h"

void *connectStorageServer(void *arg);
void *getFileInfo(void *arg);

typedef struct file_info{
    char ss_ip[COPY_SERVERS][50];
    int ns_port[COPY_SERVERS];
    int client_port[COPY_SERVERS];
    int num_files;
    char filename[1024];
    uint64_t filesize;
    struct sockaddr_in storageserver[COPY_SERVERS];
    int storageserver_socket[COPY_SERVERS];

    int deleted;
} File;

extern pthread_mutex_t file_lock;
extern File *files[10000];

extern int filecount;
extern int servercount;

typedef struct serv{
    int server_socket;
    struct sockaddr_in server_addr;
    uint64_t filesize;
} Server;

// make the threadpool thing 
extern tpool_t* thread_pool;

extern Server *NS_storage;
extern Server *NS_client;
extern Server *servers[100];



#endif