/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/mutex.h"
#include "pico/time.h"

#if !PICO_NO_HARDWARE
static_assert(sizeof(mutex_t) == 8, "");
#endif

void mutex_init(mutex_t *mtx) {
    lock_init(&mtx->core, next_striped_spin_lock_num());
    mtx->owner = -1;
    __mem_fence_release();
}

void __time_critical_func(mutex_enter_blocking)(mutex_t *mtx) {
    assert(mtx->core.spin_lock);
    bool block = true;
    do {
        uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
        if (mtx->owner < 0) {
            mtx->owner = (int8_t)get_core_num();
            block = false;
        }
        spin_unlock(mtx->core.spin_lock, save);
        if (block) {
            __wfe();
        }
    } while (block);
}

bool __time_critical_func(mutex_try_enter)(mutex_t *mtx, uint32_t *owner_out) {
    bool entered;
    uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
    if (mtx->owner < 0) {
        mtx->owner = (int8_t)get_core_num();
        entered = true;
    } else {
        if (owner_out) *owner_out = (uint32_t) mtx->owner;
        entered = false;
    }
    spin_unlock(mtx->core.spin_lock, save);
    return entered;
}

bool __time_critical_func(mutex_enter_timeout_ms)(mutex_t *mtx, uint32_t timeout_ms) {
    return mutex_enter_block_until(mtx, make_timeout_time_ms(timeout_ms));
}

bool __time_critical_func(mutex_enter_block_until)(mutex_t *mtx, absolute_time_t until) {
    assert(mtx->core.spin_lock);
    bool block = true;
    do {
        uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
        if (mtx->owner < 0) {
            mtx->owner = (int8_t)get_core_num();
            block = false;
        }
        spin_unlock(mtx->core.spin_lock, save);
        if (block) {
            if (best_effort_wfe_or_timeout(until)) {
                return false;
            }
        }
    } while (block);
    return true;
}

void __time_critical_func(mutex_exit)(mutex_t *mtx) {
    uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
    assert(mtx->owner >= 0);
    mtx->owner = -1;
    __sev();
    spin_unlock(mtx->core.spin_lock, save);
}

/*
    WizIO - Extended Functions
*/

void mutex_init_recursive(mutex_t *mtx)
{
    mutex_init(mtx);
    mtx->counter = 0; // no owner
}

void __time_critical_func(mutex_enter_blocking_recursive)(mutex_t *mtx) {
    if (mtx->owner == (int8_t)get_core_num()) {
        mtx->counter++;
        return;
    }
    mutex_enter_blocking(mtx); // new owner
    mtx->counter = 0;
}

void __time_critical_func(mutex_exit_recursive)(mutex_t *mtx) {
    if (mtx->owner == (int8_t)get_core_num()) {
        if (mtx->counter) {
            mtx->counter--;
        } else {
            mutex_exit(mtx); // no owner
            mtx->counter = 0;
        }
    }
    else {
        assert(0); // the core is not owner
    }
}

int __time_critical_func(mutex_try_enter_recursive)(mutex_t *mtx, uint32_t *owner_out)
{
    if (mtx->owner == (int8_t)get_core_num()) {
        ++mtx->counter;
        return 1; // pdPASS -> pdTRUE -> 1
    }
    bool xReturn = mutex_try_enter(mtx, owner_out); // FIX? return ... true = unowned,
    /* 
     * xReturn = xQueueSemaphoreTake( pxMutex, 0 );
     * pdPASS(1) will only be returned if the mutex was successfully obtained.  
     * The calling task may have entered the Blocked state before reaching here. 
     * https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/23f641850d2428eac3e164d6e735e6e92dc3914a/queue.c#L691
     * 
     */
    if (xReturn != false) { // FIX? ... if ( xReturn != pdFAIL(0) )
        ++mtx->counter;
    }
    return xReturn; // as newlib <sys/lock.h>
}