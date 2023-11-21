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

typedef struct serv{
    int server_socket;
    struct sockaddr_in server_addr;
    uint64_t filesize;
    pthread_mutex_t ss_lock;
} Server;

typedef struct file_info{
    Server *on_servers[COPY_SERVERS];
    char filename[1024];
    int deleted;
} File;

extern pthread_mutex_t file_lock;
extern File *files[10000];

extern int filecount;
extern int servercount;

// make the threadpool thing 
extern tpool_t* thread_pool;

extern Server *NS_storage;
extern Server *NS_client;
extern Server *servers[100];


#endif