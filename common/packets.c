#include "packets.h"
#include "buffer.h"
#include <string.h>

/**
 *  OP:READ
 *  FILENAME:filename
 *  PORT:portno
 *
 *
 *  1. Convert struct to packet
 *  add_str_header(&buffer[0], "OP:", "READ");
 *  add_str_header(&buffer[1], "FILENAME:", "filename");
 *  add_int_header(&buffer[2], "PORT", portno);
 *
 *  coalsce_buffers(buffer, 2);
 *
 *  2. Convert packet to struct
 *  (fd is where we are reading from)
 *  buf_t* operation = read_str(fd, "OP:");
 *  buf_t* filename = read_str(fd, "FILENAME:");
 *  i32 port = read_i32(fd, "PORT:");
 *  */

void add_str_header(str_t* buffer, const char* hname, const char* hvalue) {
	int len = strlen(hname) + strlen(hvalue) + 2; // 1 for NULL, 1 for \n
	buf_malloc(buffer, sizeof(char), len);
	sprintf(CAST(char, buffer->data), "%s%s\n", hname, hvalue);
	buffer->len = len - 1;
}

void add_buf_header(str_t* buffer, const char* hname, const buf_t hvalue) {
	int len = strlen(hname) + hvalue.len + 2; // 1 for NULL, 1 for \n
	buf_malloc(buffer, sizeof(char), len);
	sprintf(CAST(char, buffer->data), "%s%s\n", hname,
	        CAST(char, hvalue.data));
	buffer->len = len;
}

void add_int_header(str_t* buffer, const char* hname, const size_t hvalue) {
	char hval_str[20];
	sprintf(hval_str, "%zu", hvalue);
	add_str_header(buffer, hname, hval_str);
}

void add_i64_header(str_t* buffer, const char* hname, const i64 hvalue) {
	char hval_str[20];
	sprintf(hval_str, "%ld", hvalue);
	add_str_header(buffer, hname, hval_str);
}

void coalsce_buffers(buf_t* dest, buf_t* src_bufs) {
	size_t total_len = 0;
	size_t num_buffers = src_bufs->len;
	for (int i = 0; i < num_buffers; i++) {
		total_len += CAST(str_t, src_bufs->data)[i].len;
	}

	buf_malloc(dest, sizeof(char), total_len + 1);
	char* data = CAST(char, dest->data);
	for (int i = 0; i < num_buffers; i++) {
		strncat(data, CAST(char, CAST(str_t, src_bufs->data)[i].data),
		        CAST(str_t, src_bufs->data)[i].len);
		dest->len += CAST(str_t, src_bufs->data)[i].len;
	}
}

void readline(int fd, buf_t* buf) {
	buf_malloc(buf, sizeof(char), 32); // nice initial size

	char ch;
	ssize_t err;
	while ((err = recv(fd, &ch, sizeof(char), 0)) > 0) {
		if (buf->len == buf->capacity) {
			buf_resize(buf, 2 * buf->capacity);
		}

		if (ch == '\n') {
			CAST(char, buf->data)[buf->len++] = '\0';
			break;
		}
		CAST(char, buf->data)[buf->len++] = ch;
	}

	// TODO: error checking
}

i32 read_i32(int fd, char* hname) {
	buf_t header;
	readline(fd, &header);

	char* hval_str = strstr(CAST(char, header.data), hname);
	if (hval_str == NULL) {
		return -1;
	}

	i32 result;
	sscanf(CAST(char, header.data), "%d", &result);
	return result;
}

i64 read_i64(int fd, char* hname) {
	buf_t header;
	readline(fd, &header);

	char* hval_str = strstr(CAST(char, header.data), hname);
	if (hval_str == NULL) {
		return -1;
	}

	i64 result;
	sscanf(hval_str, "%ld", &result);
	return result;
}

buf_t* read_str(int fd, const char* hname) {
	size_t hname_len = strlen(hname);

	buf_t header;
	readline(fd, &header);

	buf_t* hval = malloc(sizeof(buf_t));
	buf_malloc(hval, sizeof(char), (header.len - hname_len + 1));
	hval->len = (header.len - hname_len + 1);

	char* hval_str = strstr(CAST(char, header.data), hname);
	if (hval_str == NULL) {
		hval->len = 0;
		return hval;
	}

	sscanf(CAST(char, header.data), "%s", CAST(char, hval->data));

	return hval;
}
