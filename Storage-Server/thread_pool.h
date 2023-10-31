#ifndef THREAD_POOL
#define THREAD_POOL

#include <stdbool.h>
#include <stddef.h>

struct tpool;
typedef struct tpool tpool_t;

typedef void (*tpool_func_t)(void *arg);

tpool_t* tpool_create (size_t num_threads);
void tpool_destroy (tpool_t* thread_pool);

int tpool_work (tpool_t* thread_pool, tpool_func_t function, void* args);
void tpool_wait (tpool_t* thread_pool);
#endif // THREAD_POOL_H_
