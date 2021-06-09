/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/mutex.h"
#include "pico/time.h"

static void mutex_init_internal(mutex_t *mtx, uint8_t recursion_state) {
    lock_init(&mtx->core, next_striped_spin_lock_num());
    mtx->owner = LOCK_INVALID_OWNER_ID;
    mtx->recursion_state = recursion_state;
    __mem_fence_release();
}

void mutex_init(mutex_t *mtx) {
    mutex_init_internal(mtx, 0);
}

void recursive_mutex_init(mutex_t *mtx) {
    mutex_init_internal(mtx, MAX_RECURSION_STATE);
}

void __time_critical_func(mutex_enter_blocking)(mutex_t *mtx) {
    assert(mtx->core.spin_lock);
    do {
        uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
        lock_owner_id_t caller = lock_get_caller_owner_id();
        if (mtx->owner == LOCK_INVALID_OWNER_ID) {
            mtx->owner = caller;
            if (mtx->recursion_state) {
                assert(mtx->recursion_state == MAX_RECURSION_STATE);
                mtx->recursion_state--;
            }
        } else if (mtx->owner == caller && mtx->recursion_state > 1) {
            mtx->recursion_state--;
        } else {
            lock_internal_spin_unlock_with_wait(&mtx->core, save);
            // spin lock already unlocked, so loop again
            continue;
        }
        spin_unlock(mtx->core.spin_lock, save);
        break;
    } while (true);
}

bool __time_critical_func(mutex_try_enter)(mutex_t *mtx, uint32_t *owner_out) {
    bool entered;
    uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
    lock_owner_id_t caller = lock_get_caller_owner_id();
    if (mtx->owner == LOCK_INVALID_OWNER_ID) {
        mtx->owner = lock_get_caller_owner_id();
        entered = true;
    } else if (mtx->owner == caller && mtx->recursion_state > 1) {
        mtx->recursion_state--;
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

bool __time_critical_func(mutex_enter_timeout_us)(mutex_t *mtx, uint32_t timeout_us) {
    return mutex_enter_block_until(mtx, make_timeout_time_us(timeout_us));
}

bool __time_critical_func(mutex_enter_block_until)(mutex_t *mtx, absolute_time_t until) {
    assert(mtx->core.spin_lock);
    do {
        uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
        lock_owner_id_t caller = lock_get_caller_owner_id();
        if (mtx->owner == LOCK_INVALID_OWNER_ID) {
            mtx->owner = caller;
        } else if (mtx->owner == caller && mtx->recursion_state > 1) {
            mtx->recursion_state--;
        } else {
            if (lock_internal_spin_unlock_with_best_effort_wait_or_timeout(&mtx->core, save, until)) {
                // timed out
                return false;
            } else {
                // not timed out; spin lock already unlocked, so loop again
                continue;
            }
        }
        spin_unlock(mtx->core.spin_lock, save);
        return true;
    } while (true);
}

void __time_critical_func(mutex_exit)(mutex_t *mtx) {
    uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
    assert(mtx->owner != LOCK_INVALID_OWNER_ID);
    if (!mtx->recursion_state) {
        mtx->owner = LOCK_INVALID_OWNER_ID;
        lock_internal_spin_unlock_with_notify(&mtx->core, save);
    } else {
        mtx->recursion_state++;
        assert(mtx->recursion_state);
        if (mtx->recursion_state == MAX_RECURSION_STATE) {
            mtx->owner = LOCK_INVALID_OWNER_ID;
            lock_internal_spin_unlock_with_notify(&mtx->core, save);
        } else {
            spin_unlock(mtx->core.spin_lock, save);
        }
    }
}

/* WizIO: Extended Functions TODO */
#if 0 
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
    if (xReturn != false) { // FIX? ... if ( xReturn != pdFAIL(0) )
        ++mtx->counter;
    }
    return xReturn; // as newlib <sys/lock.h>
}
#endif