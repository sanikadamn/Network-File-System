#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define DEFAULT_NS_PORT 3112

#include "connect_storage_server.h"
#include "client_operations.h"
#include "../error_codes.h"
