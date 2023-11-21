#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <limits.h>

#define DEFAULT_NS_SS_PORT 3112
#define DEFAULT_NS_CLIENT_PORT 3113
#define LRU_SIZE 10 

#ifndef NUM_THREADS
#define NUM_THREADS 50
#endif

#ifndef COPY_SERVERS
#define COPY_SERVERS 3
#endif

extern File *lru_cache[LRU_SIZE]; // adds files when they are accessed
extern int lru_index;
extern pthread_mutex_t lru_mutex;

#include "connect_storage_server.h"
#include "client_operations.h"
#include "pq.h"
#include "../common/error_codes.h"
#include "../common/Structures.h"
#include "../common/buffer.h"
#include "../common/packets.h"
#include "../common/new_packets.h"
#include "../common/readline.h"