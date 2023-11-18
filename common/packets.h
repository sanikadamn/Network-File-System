#ifndef PACKETS
#define PACKETS

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "buffer.h"

#define DELETE 0

// struct to send requests from ns to ss
typedef struct com{
    int type;
    char path[1024];
} Command;

#endif
