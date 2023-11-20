#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "../common/buffer.h"
#include "../common/packets.h"
#include "../common/new_packets.h"
#include "../common/serialize.h"
#include "../error_codes.h"
#include "ss_operations.h"
#include "ns_operations.h"
#include "client.h"

#define DEFAULT_CLIENT_PORT 2109
#define DEFAULT_NS_PORT 3112
#define LOCALHOST "127.0.0.1"

extern int client_ns_socket;
extern int client_ss_socket;