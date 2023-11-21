#ifndef NETWORK
#define NETWORK

#include "filemap.h"
#include "../common/thread_pool.h"

int init_connection (char* ip, char* port, int is_server);

struct listen_args {
    tpool_t* thread_pool;
    int sockfd;
};

void listen_client_connections (void* arg);

#endif // NETWORK_H_
