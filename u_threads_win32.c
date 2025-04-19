#include <stddef.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include "u_threads.h"

static void __cdecl thread_func_wrapper(void *arg)
{
    U_Thread *th = arg;
    th->func(th->arg);
}

int U_thread_create(U_Thread *th, void (*func)(void *), void *arg)
{
    uintptr_t ret;
    unsigned stack_size;
    th->func = func;
    th->arg = arg;
    th->thread = 0;

    stack_size = 1 << 22; // 4 MB
    ret = _beginthread(thread_func_wrapper, stack_size, th);

    if (ret == -1L)
        return 0;

    th->thread = (void*)ret;

    return 1;
}

int U_thread_set_name(U_Thread *th, const char *name)
{
    /* TODO implement */
    /* https://learn.microsoft.com/de-de/previous-versions/visualstudio/visual-studio-2015/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN */
    (void)th;
    (void)name;
    return 0;
}

int U_thread_join(U_Thread *th)
{
    DWORD ret;

    ret = WaitForSingleObject((HANDLE)th->thread, INFINITE);

    if (ret == WAIT_OBJECT_0)
        return 1;

    return 0;
}

void U_thread_exit(int result)
{
    (void)result;
    _endthread();
}

void U_thread_msleep(unsigned long milliseconds)
{
    if (milliseconds)
        Sleep((DWORD)milliseconds);
}

int U_thread_mutex_init(U_Mutex *m)
{
    m->mutex = (void*)CreateMutexW(NULL, FALSE, NULL);
    if (m->mutex)
        return 1;
    return 0;
}

int U_thread_mutex_destroy(U_Mutex *m)
{
    if (m->mutex)
    {
        CloseHandle(m->mutex);
        m->mutex = 0;
        return 1;
    }
    return 0;
}

int U_thread_mutex_lock(U_Mutex *m)
{
    DWORD ret;

    ret = WaitForSingleObject(m->mutex, INFINITE);
    if (ret == WAIT_OBJECT_0)
        return 1;

    return 0;
}

int U_thread_mutex_trylock(U_Mutex *m)
{
    DWORD ret;

    ret = WaitForSingleObject(m->mutex, 0);
    if (ret == WAIT_OBJECT_0)
        return 1;

    return 0;
}

int U_thread_mutex_unlock(U_Mutex *m)
{
    if (ReleaseMutex(m->mutex))
        return 1;

    return 0;
}

int U_thread_semaphore_init(U_Semaphore *s, unsigned initial_value)
{
    s->sem = (void*)CreateSemaphoreW(NULL, (LONG)initial_value, MAXLONG, NULL);
    if (s->sem)
        return 1;

    return 0;
}

int U_thread_semaphore_destroy(U_Semaphore *s)
{
    if (s->sem)
    {
        CloseHandle(s->sem);
        s->sem = 0;
        return 1;
    }

    return 0;
}

int U_thread_semaphore_wait(U_Semaphore *s)
{
    if (WaitForSingleObject(s->sem, INFINITE) == WAIT_OBJECT_0)
        return 1;

    return 0;
}

int U_thread_semaphore_post(U_Semaphore *s)
{
    if (ReleaseSemaphore(s->sem, 1, NULL))
        return 1;

    return 0;
}
