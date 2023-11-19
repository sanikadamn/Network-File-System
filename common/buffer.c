#include <stdlib.h>

#include "buffer.h"

int buf_malloc(struct buffer* b, size_t el_size, size_t len) {
	if (len < 0 || b == NULL) {
		return 1; // TODO: Custom error mechanism?
	} else {
		b->el_size = el_size;
		b->capacity = len;
		b->len = 0;
		b->data = calloc(len, el_size);
		return -(b->data != NULL); // if malloc returned NULL, it
		                           // required checking error code.
	}
}

int buf_resize(struct buffer* b, size_t cap) {
	if (b == NULL || cap < b->len) {
		return 1;
	} else {
		void* temp = realloc(b->data, cap * b->el_size);
		if (temp == NULL)
			return 1;
		b->data = temp;
		b->capacity = cap;
	}
	return 0;
}

void buf_free(struct buffer* b) {
	if (b == NULL)
		return;
	free(b->data);
	b->len = 0;
	b->capacity = 0;
}
