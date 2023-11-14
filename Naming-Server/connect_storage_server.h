#ifndef CONNECTSERVER_H
#define CONNECTSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

void *connectStorageServer(void *arg);
void *getFileInfo(void *arg);

typedef struct file_info{
    char path[1024];
    struct sockaddr_in storageserver;
} File;

extern pthread_mutex_t file_lock;
extern File files[10000];

extern int filecount;

typedef struct serv{
    int server_socket;
    struct sockaddr_in server_addr;
} Server;

extern Server *NS;

#endif