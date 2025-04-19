/*
 * Copyright (c) 2023 Manuel Pietschmann.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 *
 * Upstream repository: https://git.sr.ht/~cryo/u_threads
 */

#ifndef U_THREADS_H
#define U_THREADS_H

#ifndef U_LIBAPI
#ifdef _WIN32
  #ifdef USE_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllimport)
  #endif
  #ifdef BUILD_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllexport)
  #endif
#endif
#endif /* ! defined(U_LIBAPI) */

#ifndef U_LIBAPI
#define U_LIBAPI
#endif

#ifndef USE_PTHREADS
#ifndef _WIN32
#define USE_PTHREADS
#endif
#endif

#ifdef USE_PTHREADS
#include <pthread.h>
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif
#endif

typedef struct U_Thread
{
#ifdef USE_PTHREADS
    pthread_t thread;
#endif
#ifdef _WIN32
    void *thread;
#endif
    void (*func)(void*);
    void *arg;
} U_Thread;

typedef struct U_Mutex
{
#ifdef USE_PTHREADS
    pthread_mutex_t mutex;
#endif
#ifdef _WIN32
    void *mutex;
#endif
} U_Mutex;

typedef struct U_Semaphore
{
#ifdef USE_PTHREADS
#ifdef __APPLE__
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
#endif
#ifdef _WIN32
    void *sem;
#endif
} U_Semaphore;

#ifdef __cplusplus
extern "C" {
#endif

U_LIBAPI int U_thread_create(U_Thread *th, void (*func)(void *), void *arg);
U_LIBAPI int U_thread_set_name(U_Thread *th, const char *name);
U_LIBAPI int U_thread_join(U_Thread *th);
U_LIBAPI void U_thread_exit(int result);
U_LIBAPI void U_thread_msleep(unsigned long milliseconds);

U_LIBAPI int U_thread_mutex_init(U_Mutex *);
U_LIBAPI int U_thread_mutex_destroy(U_Mutex *);
U_LIBAPI int U_thread_mutex_lock(U_Mutex *);
U_LIBAPI int U_thread_mutex_trylock(U_Mutex *);
U_LIBAPI int U_thread_mutex_unlock(U_Mutex *);

U_LIBAPI int U_thread_semaphore_init(U_Semaphore *s, unsigned initial_value);
U_LIBAPI int U_thread_semaphore_destroy(U_Semaphore *s);
U_LIBAPI int U_thread_semaphore_wait(U_Semaphore *s);
U_LIBAPI int U_thread_semaphore_post(U_Semaphore *s);

#ifdef __cplusplus
}
#endif

#endif /* U_THREADS_H */
