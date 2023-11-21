#include <error.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "client.h"
#include "../common/globals.h"
#include "constants.h"
#include "filemap.h"
#include "../common/readline.h"

void send_status(int fd, int status_num) {
	char status_buf[30] = {0};
	sprintf(status_buf, "STATUS:%d\n", status_num);
	send(fd, status_buf, strlen(status_buf), 0);
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

    struct file_metadata* file = search_file(filename);

    if (file == NULL) {
        printf("file not found\n");
        // send_status(fd, NS_FILE_NOT_FOUND);
        return;
    }

    int file_fd = open(CAST(char, file->local_filename.data), O_RDONLY);
    if (file_fd == -1) {
        // send_status(fd,);
        perror("client read");
        return;
    }

    printf("sending data\n");
    int num_bytes = file->file_size;
    char numbytes_buffer[30] = {0};
    sprintf(numbytes_buffer, "NUMBYTES:%d\n", num_bytes);
    send(fd, numbytes_buffer, strlen(numbytes_buffer), 0);

    char* buffer[BUFSIZE] = {0};

    while (num_bytes > BUFSIZE) {
        read(file_fd, buffer, BUFSIZE);
        send(fd, buffer, BUFSIZE, 0);
        num_bytes -= BUFSIZE;
    }

    read(fd, buffer, num_bytes);
    send(fd, buffer, num_bytes, 0);

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

    struct file_metadata* f = search_file(filename);

    if (f == NULL) {

    }

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
