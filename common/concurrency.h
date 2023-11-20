#ifndef CONCURRENCY_H_
#define CONCURRENCY_H_

/**
 * Solution shamelessly stolen from Wikipedia: https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem
 *
 * All macros require x to be pointer having the following fields
 *     sem_t queue;
 *     sem_t read_lock;
 *     sem_t lock;
 *     int readcount;
 *  */


#define READER_ENTER(x)  do {                   \
        sem_wait(&((x)->queue));                \
        sem_wait(&((x)->read_lock));            \
        (x)->readcount++;                       \
        if ((x)->readcount == 1)                \
            sem_wait(&((x)->lock));             \
        sem_post(&((x)->queue));                \
        sem_post(&((x)->read_lock));            \
    } while (0);                                \

#define READER_EXIT(x) do {                     \
        sem_wait(&((x)->read_lock));            \
        ((x)->readcount)--;                     \
        if ((x)->readcount == 0)                \
            sem_post(&((x)->lock));             \
        sem_post(&((x)->read_lock));            \
    } while (0);

#define WRITER_ENTER(x) do {                    \
        sem_wait(&((x)->queue));                \
        sem_wait(&((x)->lock));                 \
        sem_post(&((x)->queue));                \
    } while (0);

#define WRITER_EXIT(x) do {                     \
    sem_post(&((x)->lock));                     \
    } while (0);


#endif // CONCURRENCY_H_
