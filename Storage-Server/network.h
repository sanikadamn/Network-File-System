#ifndef NETWORK
#define NETWORK

#include "filemap.h"
#include "thread_pool.h"

extern struct files* ss_files;

int init_connection (char* ip, char* port, int server);

void send_heartbeat (void* arg);

struct listen_args {
    tpool_t* thread_pool;
    int sockfd;
};

void listen_connections (void* arg);

#endif // NETWORK_H_
