#ifndef NETWORK
#define NETWORK

#include "filemap.h"
#include "thread_pool.h"

extern int ns_socket;
extern struct files* ss_files;

int init_ns_connection (char* ip, char* port);

void send_heartbeat ();

void listen_connections (void* arg);

#endif // NETWORK_H_
