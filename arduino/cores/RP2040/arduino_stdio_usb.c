/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef PICO_STDIO_USB

#if !defined(TINYUSB_HOST_LINKED) && !defined(TINYUSB_DEVICE_LINKED)
#include "tusb.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/stdio_usb.h"
#include "hardware/irq.h"
#include "pico/mutex.h"
#include "debug.h"

static_assert(PICO_STDIO_USB_LOW_PRIORITY_IRQ > RTC_IRQ, "");
static mutex_t stdio_usb_mutex;

static void low_priority_worker_irq(void)
{
    if (mutex_try_enter(&stdio_usb_mutex, NULL))
    {
        tud_task();
        mutex_exit(&stdio_usb_mutex);
    }
}

static int64_t timer_task(__unused alarm_id_t id, __unused void *user_data)
{
    irq_set_pending(PICO_STDIO_USB_LOW_PRIORITY_IRQ);
    return PICO_STDIO_USB_TASK_INTERVAL_US;
}

static int dbg_usb_out_chars(struct _reent *r, _PTR p, const char *buf, int length)
{
    static uint64_t last_avail_time;
    uint32_t owner;
    if (!mutex_try_enter(&stdio_usb_mutex, &owner))
    {
        if (owner == get_core_num())
            return -1; // would deadlock otherwise
        mutex_enter_blocking(&stdio_usb_mutex);
    }
    if (tud_cdc_connected())
    {
        for (int i = 0; i < length;)
        {
            int n = length - i;
            int avail = tud_cdc_write_available();
            if (n > avail)
                n = avail;
            if (n)
            {
                int n2 = tud_cdc_write(buf + i, n);
                tud_task();
                tud_cdc_write_flush();
                i += n2;
                last_avail_time = time_us_64();
            }
            else
            {
                tud_task();
                tud_cdc_write_flush();
                if (!tud_cdc_connected() ||
                    (!tud_cdc_write_available() && time_us_64() > last_avail_time + PICO_STDIO_USB_STDOUT_TIMEOUT_US))
                {
                    break;
                }
            }
        }
    }
    else
    {
        last_avail_time = 0; // reset our timeout
    }
    mutex_exit(&stdio_usb_mutex);
    return length;
}

static int dbg_usb_in_chars(struct _reent *r, _PTR p, char *buf, int length)
{
    uint32_t owner;
    if (!mutex_try_enter(&stdio_usb_mutex, &owner))
    {
        if (owner == get_core_num())
            return PICO_ERROR_NO_DATA; // would deadlock otherwise
        mutex_enter_blocking(&stdio_usb_mutex);
    }
    int rc = PICO_ERROR_NO_DATA;
    if (tud_cdc_connected() && tud_cdc_available())
    {
        int count = tud_cdc_read(buf, length);
        rc = count ? count : PICO_ERROR_NO_DATA;
    }
    mutex_exit(&stdio_usb_mutex);
    return rc;
}

void dbg_usb_init(void)
{
    tusb_init(); // initialize TinyUSB
    irq_set_exclusive_handler(PICO_STDIO_USB_LOW_PRIORITY_IRQ, low_priority_worker_irq);
    irq_set_enabled(PICO_STDIO_USB_LOW_PRIORITY_IRQ, true);
    mutex_init(&stdio_usb_mutex);
    int rc = add_alarm_in_us(PICO_STDIO_USB_TASK_INTERVAL_US, timer_task, NULL, true);
    if (rc)
    {
        stdio_drv.ctx = dbg_usb_init; // not NULL
        stdio_drv.write_r = dbg_usb_out_chars;
        stdio_drv.read_r = dbg_usb_in_chars;
        dbg_retarget(&stdio_drv);
    }
}

#else // (TINYUSB_HOST_LINKED) && (TINYUSB_DEVICE_LINKED)
#include "pico/stdio_usb.h"
#warning stdio USB was configured, but is being disabled as TinyUSB is explicitly linked
void dbg_usb_init(void) {}
#endif

#else // PICO_STDIO_USB
#include <stdbool.h>
void dbg_usb_init(void) {}
#endif