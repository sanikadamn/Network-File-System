#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>

#include <stdint.h>
#include <stdio.h>

#include "filemap.h"
#include "network.h"
#include "thread_pool.h"
#include "constants.h"

typedef uint32_t u32;

void *get_in_addr (struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int init_connection(char* ip, char* port) {
    // initialising socket data
    struct addrinfo hints = {0};
    struct addrinfo* res = NULL;
    int sockfd, new_fd;
    u32 yes = 1;

    // init hints
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // because TCP
    hints.ai_flags = AI_PASSIVE; // autofill ip address
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    // get host addr
    int err;
    do {
        err = getaddrinfo(ip, port, &hints, &res);
    } while (err == EAI_AGAIN);

    if (err < 0) {
        if (err == EAI_SYSTEM) {
            perror("error: ");
        } else {
            fprintf(stderr, "Error resolving host: errno = %d, %s\n", err, gai_strerror(err));
        }
        return -1;
    }

    fprintf(stderr, "Found domain, connecting...\n");

    // loop through all available addresses, and
    // use the first one that connects sucessfully.
    for (struct addrinfo* head = res; head != NULL; head = head->ai_next) {
        sockfd = socket(head->ai_family,
                        head->ai_socktype,
                        head->ai_protocol);

        if (sockfd == -1) continue;

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            return -1;
        }

        if (bind(sockfd, head->ai_addr, head->ai_addrlen) != -1) {
            break; // if connection sucessful: do not continue connecting
        }

        sockfd = -1;
        perror("server: bind");
        close(sockfd);
    }

    freeaddrinfo(res);

    if (sockfd == -1) {
        fprintf(stderr, "ERROR: Could not connect\nExiting...\n");
        return sockfd;
    }

    fprintf(stderr, "Connected, sockfd = %d!\n", sockfd);
    return sockfd;
}

void send_heartbeat () {
    while (1) {
        sem_wait(&ss_files->data_queue);
        sem_wait(&ss_files->data_read_lock);

        buf_t* packet = prepare_filemap_packet(*ss_files);
        if (send(ns_socket, packet->data, packet->len, 0) == -1) {
            perror("send");
        }

        char buffer[512];
        int err = recv(ns_socket, buffer, sizeof(buffer), 0);
        if (err == -1) {
            perror("recv");
            continue;
        } else if (err == 0) {
            printf("client closed connection\n");
        }

        sem_post(&ss_files->data_read_lock);
        sem_post(&ss_files->data_queue);
        sleep(10);
    }
}

void respond (void* arg) {
    int __attribute__((unused)) new_fd = *((int*) arg);
    return;
}


void listen_connections (void* arg) {
    int sockfd = *((int *)arg);
    arg = (int *)arg + 1;
    tpool_t* threadpool = ((tpool_t *) arg);

    // listens to socket
    if (listen(sockfd, DEFAULT_BACKLOG) == -1) {
        perror("server: listen");
        return;
    }

    socklen_t sin_size;
    struct sockaddr_storage recv_addr;
    char buffer[BUFSIZ];
    char ip[INET6_ADDRSTRLEN];
    int new_fd;

    while (1) {
        sin_size = sizeof(recv_addr);
        printf("server: waiting for connections...\n");
        new_fd = accept(sockfd,
                        (struct sockaddr *)&recv_addr,
                        &sin_size);
        if (new_fd == -1) {
            perror("server: accept");
            continue;
        }

        inet_ntop(recv_addr.ss_family,
                  get_in_addr((struct sockaddr*) &recv_addr),
                  ip,
                  INET6_ADDRSTRLEN);
        printf("server: received connection: %s\n", ip);

        tpool_work(threadpool, respond, (void*)new_fd);
    }
}

