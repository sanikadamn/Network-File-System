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
#include "../common/concurrency.h"
#include "../common/new_packets.h"

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
        SEND_STATUS(fd, ENOTFOUND);
        return;
    }

    int file_fd = open(CAST(char, file->local_filename.data), O_RDONLY);
    if (file_fd == -1) {
        SEND_STATUS(fd, OK);
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

    read(file_fd, buffer, num_bytes);
    send(fd, buffer, num_bytes, 0);
    close(file_fd);
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

    buf_t remote_filename;
    buf_t local_filename;
    if (f == NULL) {
        buf_malloc(&remote_filename, sizeof(char), strlen(filename) + 1);
        strcpy(CAST(char, remote_filename.data), filename);
        retrieve_local_filename(&local_filename, &remote_filename);
    } else {
        WRITER_ENTER(f);
        local_filename = f->local_filename;
    }

    printf("filename: %s\n", CAST(char, local_filename.data));
    int file_fd = open(CAST(char, local_filename.data), O_CREAT | O_WRONLY);
    if (file_fd == -1) {
        perror("write open");
        SEND_STATUS(fd, EBUSY);
        if (f != NULL) {
            WRITER_EXIT(f);
        }
        return;
    }

    int num_bytes = filesize;

    char buffer[BUFSIZE] = {0};
    while (filesize > BUFSIZE && err != -1) {
        err = recv(fd, buffer, BUFSIZE, 0);
        if (err == -1) {
            perror("server recv");
            return;
        }
        err = write(file_fd, buffer, BUFSIZE);
        if (err == -1) {
            perror("server write");
            return;
        }
        filesize -= BUFSIZE;
    }

    if (err == -1)
        return;

    err = recv(fd, buffer, filesize, 0);
    write(file_fd, buffer, filesize);
    fsync(file_fd);

    if (f == NULL) {
        add_file(CAST(char, local_filename.data), CAST(char, remote_filename.data), num_bytes, S_IRWXG | S_IRWXU | S_IRWXO);
        buf_free(&local_filename);
        buf_free(&remote_filename);
    } else {
        f->file_size = num_bytes < f->file_size ? f->file_size : num_bytes;
        WRITER_EXIT(f);
    }
    free(filename);
}


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

    struct file_metadata* f = search_file(filename);

    if (f == NULL) {
        SEND_STATUS(fd, ENOTFOUND);
    }
    else {
        SEND_STATUS(fd, OK);
        char size_buf[30];
        sprintf(size_buf, "SIZE:%zu\n", f->file_size);
        send(fd, size_buf, strlen(size_buf), 0);

        char perms_buf[30];
        sprintf(perms_buf, "PERM:%d\n", f->perms);
        send(fd, perms_buf, strlen(perms_buf), 0);
    }
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
