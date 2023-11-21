#include <error.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "client.h"
#include "../common/globals.h"

char* read_line(int fd, int max_len, int* err) {
    char* str = malloc(sizeof(char) * (max_len + 1));
    if (str == NULL) return str;
    char* head = str;

	char ch;
	while ((*err = recv(fd, &ch, sizeof(char), 0)) > 0) {
		if (ch == '\n' || (head - str) == max_len) {
            *head++ = '\0';
			break;
		}
        *head++ = ch;
	}

    int len = head - str;
    str = realloc(str, len);
    return str;
}

void respond_read (int fd) {
    printf("responding to read\n");
    const char filename_header[] = "FILENAME:";
    int err;
    char* header = read_line(fd, MAX_FILENAME_LENGTH + strlen(filename_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    char* filename = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    sscanf(header, "FILENAME:%s", filename);
    free(header);

    printf("filename received: %s\n", filename);
    send(fd, "STATUS:200\n", 12, 0);
    send(fd, "NUMBYTES:50\n", 13, 0);

    while (1) {
        send(fd, "a", 1, 0);
    }
    free(filename);
}

void respond_write (int fd) {
    printf("responding to write\n");
    const char filename_header[] = "FILENAME:";
    int err;
    char* header = read_line(fd, MAX_FILENAME_LENGTH + strlen(filename_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    char* filename = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    sscanf(header, "FILENAME:%s", filename);
    free(header);

    const char numbytes_header[] = "NUMBYTES:";
    header = read_line(fd, MAX_FILENAME_LENGTH + strlen(filename_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    printf("filename received: %s\n", filename);
    int filesize;
    sscanf(header, "NUMBYTES:%d", &filesize);
    free(header);

    printf("filename received: %s\n", filename);
    printf("filesize received: %d\n", filesize);

    char buffer[BUFSIZ];
    while (filesize < BUFSIZ && !err) {
        err = recv(fd, buffer, BUFSIZ, 0);
        printf("buffer: %s\n", buffer);
    }
    err = recv(fd, buffer, BUFSIZ, 0);
    printf("buffer: %s\n", buffer);
    free(filename);
}


// TODO: Fix!!
void respond_info (int fd) {
    printf("responding to read\n");
    const char filename_header[] = "FILENAME:";
    int err;
    char* header = read_line(fd, MAX_FILENAME_LENGTH + strlen(filename_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    char* filename = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    sscanf(header, "FILENAME:%s", filename);
    free(header);

    printf("filename received: %s\n", filename);
    send(fd, "STATUS:200\n", 12, 0);
    send(fd, "SIZE:50\n", 8, 0);
    send(fd, "PERM:50\n", 8, 0);

    free(filename);
}

void respond_client (void *args) {
    int* new_fd = (int*) args;
    int fd = *new_fd;
    free(new_fd);

    const char action_header[] = "ACTION:";
    int err;
    char* header = read_line(fd, MAX_ACTION_LENGTH + strlen(action_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    char* action = malloc(sizeof(char) * MAX_ACTION_LENGTH);
    sscanf(header, "ACTION:%s", action);
    free(header);

    printf("Action received: <%s>\n", action);
    if (strncmp(action, "read", 4) == 0) respond_read(fd);
    if (strncmp(action, "info", 4) == 0) respond_info(fd);
    if (strncmp(action, "write", 4) == 0) respond_write(fd);
    free(action);
}
