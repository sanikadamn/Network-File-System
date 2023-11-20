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

#define DEFAULT_CLIENT_PORT 2109
#define DEFAULT_NS_PORT 3112
#define LOCALHOST "127.0.0.1"