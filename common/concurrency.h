#ifndef CONCURRENCY_H_
#define CONCURRENCY_H_

/**
 * Solution shamelessly stolen from Wikipedia: https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem
 *
 * All macros require x to be pointer having the following fields
 *     pthread_rwlock_t rwlock;
 *  */


#define READER_ENTER(x)  pthread_rwlock_rdlock(&(x)->rwlock);

#define READER_EXIT(x) pthread_rwlock_unlock(&(x)->rwlock);

#define WRITER_ENTER(x) pthread_rwlock_wrlock(&(x)->rwlock);

#define WRITER_EXIT(x) pthread_rwlock_unlock(&(x)->rwlock);

#endif // CONCURRENCY_H_
