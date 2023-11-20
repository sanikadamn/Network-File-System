#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "file.h"
#include "filemap.h"
#include "network.h"
#include "packets.h"
#include "../common/buffer.h"
#include "../common/serialize.h"
#include "../common/packets.h"
#include "../common/concurrency.h"
#include "constants.h"

void respond (int fd) {
    buf_t* buffer = read_str(fd, "");
}

void respond_read (int fd) {
    buf_t* filename = read_str(fd, "FILENAME:");
    char* real_filename = deserialize_string(CAST(char, filename->data));

    i64 startoffset = read_i64(fd, "STARTOFFSET:");
    i64 num_bytes = read_i64(fd, "NUMBYTES:");

    struct file_metadata* file = search_file(real_filename);

    if (file == NULL) {
        send_404();
    } else {
        printf("file found\n");

        int fd_file = open(CAST(char, file->data), O_CREAT | O_RDONLY);
        lseek(fd_file, startoffset, SEEK_SET);

        i64 num_bytes = file->file_size;
        char buffer[BUFSIZE];
        while (num_bytes < BUFSIZE) {
            // Ideally, it would have been nice to have a tee...
            num_bytes -= BUFSIZE;
            read(fd_file, buffer, BUFSIZE, 0);
            send(fd, buffer, BUFSIZE, 0);
        }
    }
}

void respond_write (int fd) {
    buf_t* filename = read_str(fd, "FILENAME:");

    i64 startoffset = read_i64(fd, "STARTOFFSET:");
    i64 num_bytes = read_i64(fd, "NUMBYTES:");

    struct file_metadata* file = search_file(filename);
    if (file != NULL) {
        WRITER_ENTER(file);
    }
    int fd_file = open(CAST(char, filename->data), O_CREAT | O_WRONLY);
    lseek(fd_file, startoffset, SEEK_SET);

    char buffer[BUFSIZE];
    while (num_bytes < BUFSIZE) {
        // Ideally, it would have been nice to have a tee...
        num_bytes -= BUFSIZE;
        recv(fd, buffer, BUFSIZE, 0);
        write(fd_file, buffer, BUFSIZE, 0);
    }

    recv(fd, buffer, num_bytes, 0);
    write(fd_file, buffer, num_bytes, 0);
    fsync(fd);

    if (file != NULL) {
        WRITER_ENTER(ss_files);
        ss_files->changed = 1;
        WRITER_EXIT(ss_files);
        file->file_size += num_bytes;
        WRITER_EXIT(file);
    } else {
        add_file(filename, num_bytes);
    }
}

void respond_delete (int fd) {
    buf_t* filename = read_str(fd, "FILENAME:");

    struct file_metadata* file = search_file(filename);
    if (file == NULL) {
        send_404();
    } else {
        WRITER_ENTER(ss_files);
        WRITER_ENTER(file);
        file->deleted = 1;
        WRITER_EXIT(file);
        WRITER_EXIT(file);
    }
}
