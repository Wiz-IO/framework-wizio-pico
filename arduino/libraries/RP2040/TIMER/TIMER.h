/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA   

    Base: https://github.com/raspberrypi/micropython/blob/pico/ports/rp2/machine_timer.c
    Edit: Georgi Angelov  
 */

#ifndef __TIMER_CLASS_H__
#define __TIMER_CLASS_H__

#ifdef __cplusplus

#include "pico/time.h"
#define ALARM_ID_INVALID (-1)
#define TIMER_MODE_ONE_SHOT (0)
#define TIMER_MODE_PERIODIC (1)

static int64_t _alarm_callback(alarm_id_t id, void *user_data);

class TIMERClass
{
private:
    struct alarm_pool *pool;

public:
    uint32_t mode;
    void (*cb)(int id);
    uint64_t delta_us;
    int alarm_id;

    TIMERClass(uint32_t _mode, void (*callback)(int))
    {
        mode = _mode;
        cb = callback;
        pool = alarm_pool_get_default();
    }

    bool begin(uint32_t freq)
    {
        delta_us = 1000000 / freq;
        if (delta_us < 1)
            delta_us = 1;
        alarm_id = alarm_pool_add_alarm_in_us(pool, delta_us, _alarm_callback, this, true);
        return (alarm_id != -1);
    }

    bool begin(uint32_t period, uint32_t tick_hz)
    {
        delta_us = (uint64_t)period * 1000000 / tick_hz;
        if (delta_us < 1)
            delta_us = 1;
        alarm_id = alarm_pool_add_alarm_in_us(pool, delta_us, _alarm_callback, this, true);
        return (alarm_id != -1);
    }

    void end() { cancel_alarm(alarm_id); }
};

static int64_t _alarm_callback(alarm_id_t id, void *user_data)
{
    TIMERClass *p = (TIMERClass *)user_data;
    if (p->cb)
        p->cb(p->alarm_id);
    if (p->mode == TIMER_MODE_ONE_SHOT)
        return 0;
    else
        return -p->delta_us;
}

#endif // __cplusplus
#endif // CLASS_H