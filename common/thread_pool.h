#ifndef THREAD_POOL
#define THREAD_POOL

// Borrows from https://nachtimwald.com/2019/04/12/thread-pool-in-c/
// Sets up a threadpool

#include <stdbool.h>
#include <stddef.h>

struct tpool;
typedef struct tpool tpool_t;

typedef void (*tpool_func_t)(void *arg);


/* Creates a new thread pool with num_threads threads.
** Work is assigned dynamically to threads using tpool_work. Each thread has a worker function that
** constantly in the background. On addition of the work, the worker function pauses and calls the
** function needed. Once the function returns, the background worker cleans up resources.
*/
tpool_t* tpool_create (size_t num_threads);

/*
** Destroys existing thread_pool
*/
void tpool_destroy (tpool_t* thread_pool);

/*
 * Used to add work to a thread. Is invoked just like pthread_create.
 */
int tpool_work (tpool_t* thread_pool, tpool_func_t function, void* args);

/*
 * Waits for threads to exit.
 */
void tpool_wait (tpool_t* thread_pool);
#endif // THREAD_POOL_H_
