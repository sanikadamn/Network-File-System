#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define DEFAULT_NS_SS_PORT 3112
#define DEFAULT_NS_CLIENT_PORT 3113

#ifndef NUM_THREADS
#define NUM_THREADS 50
#endif

#include "connect_storage_server.h"
#include "client_operations.h"
#include "../error_codes.h"
#include "../common/Structures.h"
#include "../common/buffer.h"
#include "../common/packets.h"
#include "../common/new_packets.h"