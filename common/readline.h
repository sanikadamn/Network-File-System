#ifndef READLINE_H_
#define READLINE_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

char* read_line(int fd, int max_len, int* err);

#endif // READLINE_H_
