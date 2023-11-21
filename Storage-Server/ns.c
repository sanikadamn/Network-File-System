#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ns.h"

#include "../common/concurrency.h"
#include "filemap.h"
#include "../common/packets.h"
#include "../common/serialize.h"
#include "constants.h"
#include "../common/new_packets.h"
#include "../common/readline.h"

pthread_mutex_t ns_lock;
pthread_once_t once_control = PTHREAD_ONCE_INIT;

void init_ns_lock () {
    pthread_mutex_init(&ns_lock, NULL);}

void prepare_filemap_packet(struct files file_map, struct buffer* buf) {
	READER_ENTER(&file_map);
	buf_t lines;

	// The 4 extra lines are for headers:
	// - IP address
	// - NS Port
	// - Client Port
	// - Number of files
	// Each file is then listed, with the serialised filename and size each
	// taking up a line
	buf_malloc(&lines, sizeof(str_t),
	           2 * file_map.files.len +
	               4); // See packet format for magic numbers

	// Allocate headers
	add_str_header(&CAST(str_t, lines.data)[0], "IP:", net_details.ss_ip);
	add_str_header(&CAST(str_t, lines.data)[1],
	               "NPORT:", net_details.ns_port);
	add_str_header(&CAST(str_t, lines.data)[2],
	               "CPORT:", net_details.client_port);
	add_int_header(&CAST(str_t, lines.data)[3],
	               "NUMFILES:", file_map.files.len);

	for (size_t i = 0; i < file_map.files.len; i++) {
		buf_t* buffer = serialize_buffer(
		    CAST(struct file_metadata, file_map.files.data)[i]
		        .remote_filename);
		add_buf_header(&CAST(str_t, lines.data)[2 * i + 4],
		               "FILENAME:", *buffer);
		buf_free(buffer);
		add_i64_header(
		    &CAST(str_t, lines.data)[2 * i + 5], "FILESIZE:",
		    CAST(struct file_metadata, file_map.files.data)[i]
		        .file_size);
	}

	lines.len = 2 * file_map.files.len + 4;
	coalsce_buffers(buf, &lines);
	READER_EXIT(&file_map);
}

void create (int ns_socket) {
    printf("responding to create\n");
    const char filename_header[] = "FILENAME:";
    int err;
    char* header = read_line(ns_socket, MAX_FILENAME_LENGTH + strlen(filename_header) + 1, &err);
    if (err == -1)
        perror("client respond");

    char* filename = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    sscanf(header, "FILENAME:%s", filename);
    free(header);

    printf("filename received: %s\n", filename);
    send(ns_socket, "STATUS:200\n", 12, 0);
    send(ns_socket, "SIZE:50\n", 8, 0);

    free(filename);
}

void delete (int ns_socket) {

}

void respond_copyin (int ns_socket) {
    buf_t* filename = read_str(ns_socket, "FILENAME:");
    i64 num_bytes = read_i64(ns_socket, "NUMBYTES:");

    struct file_metadata* file = search_file(filename);
    if (file != NULL) {
        WRITER_ENTER(file);
    }

    char* remote_file;

    int fd = open(CAST(char, filename->data), O_CREAT | O_WRONLY);

    char buffer[BUFSIZE];
    while (num_bytes < BUFSIZE) {
        // Ideally, it would have been nice to have a tee...
        num_bytes -= BUFSIZE;
        recv(ns_socket, buffer, BUFSIZE, 0);
        write(fd, buffer, BUFSIZE);
    }

    recv(ns_socket, buffer, num_bytes, 0);
    write(fd, buffer, num_bytes);

    fsync(fd);

    if (file != NULL) {
        file->file_size += num_bytes;
        file->updated = 1;
        WRITER_EXIT(file);
    } else {
        // add_file(filename,; num_bytes);
    }

    free(file);
    buf_free(filename);
    free(filename);
}

void respond_copyout (int ns_socket) {
    buf_t* filename = read_str(ns_socket, "FILENAME:");

    struct file_metadata* file = search_file(filename);
    if (file != NULL) {
        return;
    } else {
        buf_t headers;
        buf_malloc(&headers, sizeof(buf_t), 2);

        add_buf_header(&CAST(str_t, headers.data)[0], "FILENAME:", file->remote_filename);
        add_i64_header(&CAST(str_t, headers.data)[1], "SIZE:", file->file_size);
        buf_t total;
        coalsce_buffers(&total, &headers);
        send(ns_socket, CAST(char, headers.data), headers.len, 0);

        int fd = open(CAST(char, file->local_filename.data), O_CREAT | O_RDONLY);
        i64 num_bytes = file->file_size;
        char buffer[BUFSIZE];
        while (num_bytes < BUFSIZE) {
            // Ideally, it would have been nice to have a tee...
            num_bytes -= BUFSIZE;
            read(fd, buffer, BUFSIZE);
            send(ns_socket, buffer, BUFSIZE, 0);
        }
    }
    buf_free(filename);
    free(filename);
}


void send_heartbeat(void* arg) {
	int ns_socket = (int)arg;

    pthread_once(&once_control, init_ns_lock);

	printf("IMPORTANT: starting heartbeat sending\n");

	while (1) {
		READER_ENTER(ss_files);

		printf("sending heartbeat\n");

		buf_t packet;
        prepare_filemap_packet(*ss_files, &packet);
        pthread_mutex_lock(&ns_lock);
		size_t err = send(ns_socket, (char*)packet.data,
		                  packet.len, 0);
        pthread_mutex_unlock(&ns_lock);
        buf_free(&packet);

		if (err == -1) {
			perror("ns send");
		}

		READER_EXIT(ss_files);
		sleep(10); // NOLINT(concurrency-mt-unsafe)
	}
	return;
}

void listen_ns (void* arg) {
    int ns_socket = (int)arg;

    pthread_once(&once_control, init_ns_lock);

    size_t err;
    char buffer[512];
    while ((err = recv(ns_socket, buffer, sizeof(buffer), MSG_PEEK))) {
        pthread_mutex_lock(&ns_lock);
        char* op = read_str(ns_socket, "REQUEST:");

        if (strcmp(op, "COPYIN") != 0) respond_copyin(ns_socket);
        if (strcmp(op, "COPYOUT") != 0) respond_copyout(ns_socket);

        pthread_mutex_unlock(&ns_lock);
    }
}
