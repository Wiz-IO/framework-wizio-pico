////////////////////////////////////////////////////////////////////////////////////////
//
//      2021 Georgi Angelov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////////////

#include "sys/lock.h"
#include "hardware/sync.h"

#ifdef USE_LOCK

#define LOCK_DBG

#define LOCK_SIMPLE 1

#if LOCK_SIMPLE > 0
static struct __lock s_common_mutex = {0};
static struct __lock s_common_recursive_mutex = {0};
extern struct __lock __attribute__((alias("s_common_recursive_mutex"))) __lock___sinit_recursive_mutex;
extern struct __lock __attribute__((alias("s_common_recursive_mutex"))) __lock___malloc_recursive_mutex;
extern struct __lock __attribute__((alias("s_common_recursive_mutex"))) __lock___env_recursive_mutex;
extern struct __lock __attribute__((alias("s_common_recursive_mutex"))) __lock___sfp_recursive_mutex;
extern struct __lock __attribute__((alias("s_common_recursive_mutex"))) __lock___atexit_recursive_mutex;
extern struct __lock __attribute__((alias("s_common_mutex"))) __lock___at_quick_exit_mutex;
extern struct __lock __attribute__((alias("s_common_mutex"))) __lock___tz_mutex;
extern struct __lock __attribute__((alias("s_common_mutex"))) __lock___dd_hash_mutex;
extern struct __lock __attribute__((alias("s_common_mutex"))) __lock___arc4random_mutex;
#else
struct __lock __lock___sinit_recursive_mutex = {0};
struct __lock __lock___sfp_recursive_mutex = {0};
struct __lock __lock___atexit_recursive_mutex = {0};
struct __lock __lock___at_quick_exit_mutex = {0};
struct __lock __lock___malloc_recursive_mutex = {0};
struct __lock __lock___env_recursive_mutex = {0};
struct __lock __lock___tz_mutex = {0};
struct __lock __lock___dd_hash_mutex = {0};
struct __lock __lock___arc4random_mutex = {0};
#endif

#define LOCK_ENTER_CRITICAL() uint32_t _prim = save_and_disable_interrupts()
#define LOCK_EXIT_CRITICAL() restore_interrupts(_prim)

void _lock_init(_lock_t *lock)
{
    LOCK_ENTER_CRITICAL();
    if (*lock)
    {
        /* Lock already initialised */
    }
    else
    {
#ifndef USE_FREERTOS
        *lock = (_lock_t)calloc(1, sizeof(struct __lock));
        mutex_init(&(*lock)->m);
#else
        *lock = (_lock_t)xSemaphoreCreateMutex();
#endif
    }
    LOCK_EXIT_CRITICAL();
}

void _lock_init_recursive(_lock_t *lock)
{
    LOCK_ENTER_CRITICAL();
    if (*lock)
    {
        /* Lock already initialised */
    }
    else
    {
#ifndef USE_FREERTOS
        *lock = (_lock_t)calloc(1, sizeof(struct __lock));
        mutex_init(&(*lock)->m);
        LOCK_DBG("[[[INIT]]] %p [ %d ] c[ %d ]\n", *lock, (*lock)->m.owner, (*lock)->m.counter);
#else
        *lock = (_lock_t)xSemaphoreCreateRecursiveMutex();
#endif
    }
    LOCK_EXIT_CRITICAL();
}

void _lock_close(_lock_t *lock)
{
    LOCK_ENTER_CRITICAL();
    if (*lock)
    {
#ifndef USE_FREERTOS
        free(*lock);
        *lock = NULL;
#else
        vSemaphoreDelete((SemaphoreHandle_t)(*lock));
        *lock = NULL;
#endif
    }
    LOCK_EXIT_CRITICAL();
}

void _lock_close_recursive(_lock_t *lock)
{
    LOCK_DBG("[[[CLOSE]]] %p\n", *lock);
    if (*lock)
    {
#ifndef USE_FREERTOS
        free(*lock);
        *lock = NULL;
#else
        vSemaphoreDelete((SemaphoreHandle_t)(*lock));
        *lock = NULL;
#endif
    }
}

void _lock_acquire(_lock_t *lock)
{
#ifndef USE_FREERTOS
    mutex_enter_blocking(&(*lock)->m);
#else
    xSemaphoreTake((SemaphoreHandle_t)(*lock), portMAX_DELAY);
#endif
}

void _lock_acquire_recursive(_lock_t *lock)
{
#ifndef USE_FREERTOS
    //LOCK_DBG("[TAKE+] %p [ %d ] c[ %d ]\n", *lock, (*lock)->m.owner, (*lock)->m.counter);
    mutex_enter_blocking_recursive(&(*lock)->m);
    LOCK_DBG("[TAKE] %p [ %d ] c[ %d ]\n", *lock, (*lock)->m.owner, (*lock)->m.counter);
#else
    xSemaphoreTakeRecursive((SemaphoreHandle_t)(*lock), portMAX_DELAY);
#endif
}

int _lock_try_acquire(_lock_t *lock)
{
#ifndef USE_FREERTOS
    return mutex_try_enter(&(*lock)->m, NULL); // FIX? ... true = unowned
#else
    return xSemaphoreTake((SemaphoreHandle_t)(*lock), 0);
#endif
}

int _lock_try_acquire_recursive(_lock_t *lock)
{
#ifndef USE_FREERTOS
    return mutex_try_enter_recursive(&(*lock)->m, NULL);
#else
    return xSemaphoreTakeRecursive((SemaphoreHandle_t)(*lock), 0);
#endif
}

void _lock_release(_lock_t *lock)
{
#ifndef USE_FREERTOS
    mutex_exit(&(*lock)->m);
#else
    xSemaphoreGive((SemaphoreHandle_t)(*lock));
#endif
}

void _lock_release_recursive(_lock_t *lock)
{
#ifndef USE_FREERTOS

    //LOCK_DBG("[GIVE+] %p [ %d ] c[ %d ]\n", *lock, (*lock)->m.owner, (*lock)->m.counter);
    mutex_exit_recursive(&(*lock)->m);
    LOCK_DBG("[GIVE] %p [ %d ] c[ %d ]\n", *lock, (*lock)->m.owner, (*lock)->m.counter);
#else
    xSemaphoreGiveRecursive((SemaphoreHandle_t)(*lock));
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1
void __retarget_lock_init(_LOCK_T *lock)
{
    *lock = NULL;
    _lock_init(lock);
}

void __retarget_lock_init_recursive(_LOCK_T *lock)
{
    *lock = NULL;
    _lock_init_recursive(lock);
}

void __retarget_lock_close(_LOCK_T lock)
{
    _lock_close(&lock);
}
void __retarget_lock_close_recursive(_LOCK_T lock) __attribute__((alias("__retarget_lock_close")));

void __retarget_lock_acquire(_LOCK_T lock)
{
    _lock_acquire(&lock);
}

void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    _lock_acquire_recursive(&lock);
}

int __retarget_lock_try_acquire(_LOCK_T lock)
{
    return _lock_try_acquire(&lock);
}

int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return _lock_try_acquire_recursive(&lock);
}

void __retarget_lock_release(_LOCK_T lock)
{
    _lock_release(&lock);
}

void __retarget_lock_release_recursive(_LOCK_T lock)
{
    _lock_release_recursive(&lock);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void init_locks(void)
{
#ifndef USE_FREERTOS

#if LOCK_SIMPLE > 0
    mutex_init(&s_common_mutex.m);
    mutex_init(&s_common_recursive_mutex.m);
#else
    mutex_init(&__lock___sinit_recursive_mutex.m);
    mutex_init(&__lock___sfp_recursive_mutex.m);
    mutex_init(&__lock___atexit_recursive_mutex.m);
    mutex_init(&__lock___malloc_recursive_mutex.m);
    mutex_init(&__lock___env_recursive_mutex.m);

    mutex_init(&__lock___at_quick_exit_mutex.m);
    mutex_init(&__lock___tz_mutex.m);
    mutex_init(&__lock___dd_hash_mutex.m);
    mutex_init(&__lock___arc4random_mutex.m);
#endif

#else

#if LOCK_SIMPLE > 0
    xSemaphoreCreateMutexStatic((struct xSTATIC_QUEUE *)&s_common_mutex);
    xSemaphoreCreateRecursiveMutexStatic((struct xSTATIC_QUEUE *)&s_common_recursive_mutex);
#else
    xSemaphoreCreateRecursiveMutexStatic((struct xSTATIC_QUEUE *)&__lock___sinit_recursive_mutex);
    xSemaphoreCreateRecursiveMutexStatic((struct xSTATIC_QUEUE *)&__lock___sfp_recursive_mutex);
    xSemaphoreCreateRecursiveMutexStatic((struct xSTATIC_QUEUE *)&__lock___atexit_recursive_mutex);
    xSemaphoreCreateRecursiveMutexStatic((struct xSTATIC_QUEUE *)&__lock___malloc_recursive_mutex);
    xSemaphoreCreateRecursiveMutexStatic((struct xSTATIC_QUEUE *)&__lock___env_recursive_mutex);

    xSemaphoreCreateMutexStatic((struct xSTATIC_QUEUE *)&__lock___at_quick_exit_mutex);
    xSemaphoreCreateMutexStatic((struct xSTATIC_QUEUE *)&__lock___tz_mutex);
    xSemaphoreCreateMutexStatic((struct xSTATIC_QUEUE *)&__lock___dd_hash_mutex);
    xSemaphoreCreateMutexStatic((struct xSTATIC_QUEUE *)&__lock___arc4random_mutex);
#endif

#endif
}

#endif // USE_LOCK