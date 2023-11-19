#ifndef PACKETS
#define PACKETS

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "buffer.h"

#define DELETE 0

// struct to send requests from ns to ss
typedef struct com{
    int type;
    char path[1024];
} Command;


typedef int32_t i32;
typedef int64_t i64;

/**
 * Constructing HTTP-like packets that have headers with the form
 *
 * hname: hvalue
 *
 *  */

/*
 * All add_TYPE_header read header values that take the type TYPE. More specifically,
 * if hname is passed as a parameter, the following header extracts the hvalue
 * part of the header into a variable with type TYPE.
 *
 *      <hname><hvalue>
 *
 * (here, the angular brackets are for illustration)
 *  */
void add_str_header (str_t* buffer, const char* hname, const char* hvalue);
void add_buf_header (str_t* buffer, const char* hname, const buf_t hvalue);
void add_int_header (str_t* buffer, const char* hname, const size_t hvalue);
void add_i64_header (str_t* buffer, const char* hname, const i64 hvalue);

/*
 * Merge multiple buffers into one
 *  */
void coalsce_buffers (buf_t* dest, buf_t* src_bufs);


/**
 * Parsing HTTP-like packets that have headers in the form
 *
 * hname: hvalue
 *
 * All functions take in a file descriptor (that connects to the socket).
 *  */

/*
 * Assumes that the file descriptor (in this case, pointing to a socket), is not rewindable.
 * Therefore, to read line by line, it waits for a new line separator (\n), and breaks on
 * encountering it.
 *   */
void readline (int fd, buf_t* buffer);

/*
 * All read_TYPE read header values that take the type TYPE. More specifically,
 * if hname is passed as a parameter, the following header extracts the hvalue
 * part of the header into a variable with type TYPE.
 *
 *      <hname><hvalue>
 *
 * (here, the angular brackets are for illustration)
 *  */
i32 read_i32 (int fd, char* hname);
i64 read_64 (int fd, char* hname);
buf_t* read_str (int fd, const char* hname);
#endif
