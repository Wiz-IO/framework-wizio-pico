#if 0 /* TODO */

#include <stdlib.h>
#include <errno.h>
#include <sys/reent.h>

//#undef _RETARGETABLE_LOCKING
//#define _RETARGETABLE_LOCKING 0
#include <sys/lock.h>

S_Mutex __lock___sinit_recursive_mutex;
S_Mutex __lock___sfp_recursive_mutex;
S_Mutex __lock___atexit_recursive_mutex;
S_Mutex __lock___at_quick_exit_mutex;
S_Mutex __lock___malloc_recursive_mutex;
S_Mutex __lock___env_recursive_mutex;
S_Mutex __lock___tz_mutex;
S_Mutex __lock___dd_hash_mutex;
S_Mutex __lock___arc4random_mutex;

void __retarget_lock_init(_LOCK_T *plock) // __time_critical_func
{
    *plock = calloc(1, sizeof(S_Mutex));
    mutex_init(&(*plock)->pico);
    (*plock)->cnt++;
}

void __retarget_lock_init_recursive(_LOCK_T *plock)
{

    *plock = calloc(1, sizeof(S_Mutex));
    mutex_init(&(*plock)->pico);
    (*plock)->cnt++;
}

void __retarget_lock_close(_LOCK_T lock)
{
    lock->cnt--;
    free(lock);
}

void __retarget_lock_close_recursive(_LOCK_T lock)
{
    lock->cnt--;
    free(lock);
}

void __retarget_lock_acquire(_LOCK_T lock)
{
    //mutex_enter_blocking(&lock->pico);
}

void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    mutex_enter_blocking(&lock->pico);
}

void __retarget_lock_release(_LOCK_T lock)
{
    mutex_exit(&lock->pico);
}

void __retarget_lock_release_recursive(_LOCK_T lock)
{
    mutex_exit(&lock->pico);
}

int __retarget_lock_try_acquire(_LOCK_T lock)
{
    return 0;
}

int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return 0;
}

#endif

/* LOAD LIB */
void newlib_locks_init(void) {}
