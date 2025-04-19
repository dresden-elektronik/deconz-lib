#include <time.h> /* nanosleep */
#include <errno.h>
#include "u_threads.h"

static void* thread_func_wrapper(void *arg)
{
    U_Thread *th = arg;
    th->func(th->arg);
    return 0;
}

int U_thread_create(U_Thread *th, void (*func)(void *), void *arg)
{
    int ret;
    th->func = func;
    th->arg = arg;

    ret = pthread_create(&th->thread, NULL, thread_func_wrapper, th);

	if (ret != 0)
		return 0;

    return 1;
}

int U_thread_set_name(U_Thread *th, const char *name)
{
    /* Platform specific:
     * Linux
     *   int pthread_setname_np(pthread_t thread, const char *name);
     * NetBSD: name + arg work like printf(name, arg)
     *   int pthread_setname_np(pthread_t thread, const char *name, void *arg);
     * FreeBSD & OpenBSD: function name is slightly different, and has no return value
     *   void pthread_set_name_np(pthread_t tid, const char *name);
     * Mac OS X: must be set from within the thread (can't specify thread ID)
     *   int pthread_setname_np(const char*);
     */
#ifdef __LINUX__
    if (0 == pthread_setname_np(th->thread, name))
        return 1;
#endif
#ifdef __APPLE__
    if (0 == pthread_setname_np(name))
        return 1;
#endif
    return 0;
}

int U_thread_join(U_Thread *th)
{
    int ret;
    void *result;

    ret = pthread_join(th->thread, &result);

    if (ret != 0)
        return 0;

    return 1;
}

void U_thread_exit(int result)
{
    (void)result;
    pthread_exit(0);
}

void U_thread_msleep(unsigned long milliseconds)
{
    int res;
    struct timespec ts;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

int U_thread_mutex_init(U_Mutex *m)
{
    if (pthread_mutex_init(&m->mutex, 0) == 0)
        return 1;

    return 0;
}

int U_thread_mutex_destroy(U_Mutex *m)
{
    if (pthread_mutex_destroy(&m->mutex) == 0)
        return 1;

    return 0;
}

int U_thread_mutex_lock(U_Mutex *m)
{
    if (pthread_mutex_lock(&m->mutex) == 0)
        return 1;

    return 0;
}

int U_thread_mutex_trylock(U_Mutex *m)
{
    if (pthread_mutex_trylock(&m->mutex) == 0)
        return 1;

    return 0;
}

int U_thread_mutex_unlock(U_Mutex *m)
{
    if (pthread_mutex_unlock(&m->mutex) == 0)
        return 1;

    return 0;
}

int U_thread_semaphore_init(U_Semaphore *s, unsigned initial_value)
{
#ifdef __APPLE__
    s->sem = dispatch_semaphore_create((long)initial_value);
    if (s->sem != 0)
        return 1;
#else
    int pshared;

    pshared = 0;
    if (sem_init(&s->sem, pshared, initial_value) == 0)
        return 1;
#endif

    return 0;
}

int U_thread_semaphore_destroy(U_Semaphore *s)
{
#ifdef __APPLE__
    /* no destroy function?! */
    return 1;
#else
    if (sem_destroy(&s->sem) == 0)
        return 1;

    return 0;
#endif
}

int U_thread_semaphore_wait(U_Semaphore *s)
{
#ifdef __APPLE__
    dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
#else
    int r;

    do {
        r = sem_wait(&s->sem);
    } while (r == -1 && errno == EINTR);
#endif
    return 1;
}

int U_thread_semaphore_post(U_Semaphore *s)
{
#ifdef __APPLE__
    dispatch_semaphore_signal(s->sem);
    return 1;
#else
    if (sem_post(&s->sem) == 0)
        return 1;
    return 0;
#endif
}
