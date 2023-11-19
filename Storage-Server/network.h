#ifndef NETWORK
#define NETWORK

#include "filemap.h"
#include "../common/thread_pool.h"

struct network {
    char ss_ip[50];
    char ns_ip[50];
    char client_port[5];
    char ns_port[5];
};

extern struct network net_details;

extern struct files* ss_files;

int init_connection (char* ip, char* port, int is_server);

void send_heartbeat (void* arg);

struct listen_args {
    tpool_t* thread_pool;
    int sockfd;
};

void listen_connections (void* arg);

/*
** Prepare file maps into a packet that can be sent over to the naming server
*/
void prepare_filemap_packet (struct files file_map, buf_t* buffer);

#endif // NETWORK_H_
