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
