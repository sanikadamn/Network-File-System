#ifndef __CLIENT_H_
#define __CLIENT_H_

extern int client_ns_socket;
extern int client_ss_socket;
extern struct sockaddr_in client_addr;

void init_connection(char *address, int port);
char* read_line(int fd, int max_len);

// read write info

#endif
