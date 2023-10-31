#include <bits/pthreadtypes.h>
#include <stdlib.h>
#include <pthread.h>

#include "thread_pool.h"

typedef struct tpool_work {
    struct tpool_work* next;
    tpool_func_t func;
    void* args;
} tpool_work_t;


struct tpool {
    tpool_work_t* head;
    tpool_work_t* tail;

    pthread_mutex_t work_mutex;
    pthread_cond_t  work_cond;
    pthread_cond_t  working_cond;

    size_t working_thread_cnt;
    size_t thread_cnt;

    size_t stop;
};

tpool_work_t* work_malloc (tpool_func_t func, void* args) {
    tpool_work_t* work = malloc(sizeof(tpool_work_t));

    if (work == NULL) return NULL;

    work->func = func;
    work->args = args;
    work->next = NULL;
    return work;
}

void tpool_worker (void* arg) {
    tpool_t* thread_pool = arg;

    while (1) {
        pthread_mutex_lock(&(thread_pool->work_mutex));

        // Waits for work
        while (thread_pool->head == NULL && !thread_pool->stop)
            pthread_cond_wait(&(thread_pool->work_cond), &(thread_pool->work_mutex));

        tpool_work_t* work = thread_pool->head;
        thread_pool->head = work->next;
        if (work->next == NULL) {
            thread_pool->tail = NULL;
        }

        thread_pool->working_thread_cnt++;

        pthread_mutex_unlock(&(thread_pool->work_mutex));

        // Performs work
        if (work != NULL) {
            work->func(work->args);
            free(work);
        }

        // Finish work
        pthread_mutex_lock(&(thread_pool->work_mutex));
        thread_pool->working_thread_cnt--;

        // All pending work is complete: Signal for more work
        if (!thread_pool->stop && thread_pool->working_thread_cnt == 0 && thread_pool->head == NULL) {
            pthread_cond_signal(&(thread_pool->work_cond));
        }
        pthread_mutex_unlock(&(thread_pool->work_mutex));
    }

    // No longer in the thread pool
    thread_pool->thread_cnt--;
    pthread_cond_signal(&(thread_pool->work_cond));
}

int tpool_work (tpool_t *tp, tpool_func_t function, void *args) {
    if (tp == NULL) return 1;

    tpool_work_t* work = work_malloc(function, args);
    if (work == NULL) return 1;

    pthread_mutex_lock(&(tp->work_mutex));
    if (tp->tail == NULL) {
        tp->head = work;
        tp->tail = work;
    } else {
        tp->tail->next = work;
        tp->tail = work;
    }
    pthread_mutex_unlock(&(tp->work_mutex));

    pthread_cond_broadcast(&(tp->work_cond));
    return 0;
}

void tpool_wait (tpool_t *tp) {
    if (tp == NULL) return;

    pthread_mutex_lock(&(tp->work_mutex));
    while (1) {
        if ((!tp->stop && tp->working_thread_cnt == 0) || (tp->stop && tp->thread_cnt != 0)) {
            pthread_cond_wait(&(tp->working_cond), &(tp->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tp->work_mutex));
}

tpool_t* tpool_create (size_t num_threads) {
    if (num_threads == 0)
        num_threads = 5;

    tpool_t* tpool = malloc(sizeof(tpool_t));
    if (tpool == NULL) return NULL;

    tpool->head = NULL;
    tpool->tail = NULL;
    tpool->stop = 0;

    tpool->thread_cnt = num_threads;
    tpool->working_thread_cnt = 0;

    pthread_mutex_init(&(tpool->work_mutex), NULL);
    pthread_cond_init(&(tpool->work_cond), NULL);
    pthread_cond_init(&(tpool->working_cond), NULL);

    pthread_t thread;
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&thread, NULL, (void *(*)(void *))tpool_worker, tpool);
        pthread_detach(thread);
    }

    return tpool;
}

void tpool_destroy (tpool_t* tp) {
    if (tp == NULL) return;

    pthread_mutex_lock(&(tp->work_mutex));
    tpool_work_t *work = tp->head;
    tpool_work_t *temp;

    while (work != NULL) {
        temp = work->next;
        free(work);
        work = temp;
    }

    tp->stop = 1;
    pthread_cond_wait(&(tp->work_cond), &(tp->work_mutex));
    pthread_mutex_lock(&(tp->work_mutex));

    tpool_wait(tp);

    pthread_mutex_destroy(&(tp->work_mutex));
    pthread_cond_destroy(&(tp->work_cond));
    pthread_cond_destroy(&(tp->working_cond));

    free(tp);
}
