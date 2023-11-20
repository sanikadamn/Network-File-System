#ifndef __BUFFER_H_
#define __BUFFER_H_

#include <stddef.h>

struct buffer {
    size_t el_size; // element size
    size_t len;
    size_t capacity;
    void*  data;
};

// Casts void data into the required type so that pointer arithmetic etc occurs naturally
#define CAST(T, X) ((T*)(X))

typedef struct buffer buf_t;
typedef struct buffer str_t;

int buf_malloc (struct buffer *buffer, size_t el_size, size_t capacity);

int buf_resize (struct buffer* buffer, size_t new_capacity);

void buf_cpy (struct buffer* dest, struct buffer*  src);

void buf_free (struct buffer *buffer);
#endif // __BUFFER_H_
