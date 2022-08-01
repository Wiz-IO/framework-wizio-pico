////////////////////////////////////////////////////////////////////////////////////////
//      2021 Georgi Angelov
//
//      Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
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

#ifndef _SERIAL_USB_H_
#define _SERIAL_USB_H_

#ifdef __cplusplus

#include "HardwareSerial.h"
#include "hardware/irq.h"
#include "pico/time.h"
#include "tusb.h"

class SerialUSB : public HardwareSerial
{
private:
    bool _running;
    mutex_t _usb_mutex;

public:
    SerialUSB()
    {
        mutex_exit(&_usb_mutex);
        _running = false;
    }

    ~SerialUSB() { end(); }

    void begin(unsigned long baud, bool retarget = false)
    {
        begin(baud, 0, retarget);
    }

    void begin(unsigned long baud, int config, bool retarget = false)
    {
        end();
        tusb_init(); // initialize TinyUSB

#ifndef PICO_STDIO_USB_LOW_PRIORITY_IRQ
#define PICO_STDIO_USB_LOW_PRIORITY_IRQ 31 // SDK 140
#endif

        irq_set_exclusive_handler(PICO_STDIO_USB_LOW_PRIORITY_IRQ, SerialUSB_irq);
        irq_set_enabled(PICO_STDIO_USB_LOW_PRIORITY_IRQ, true);
        mutex_init(&_usb_mutex);
        _running = add_alarm_in_us(PICO_STDIO_USB_TASK_INTERVAL_US, SerialUSB_timer_task, NULL, true);
    }

    void end(void) {}

    inline size_t write(uint8_t c) { return write(&c, 1); }

    size_t write(const uint8_t *buf, size_t size)
    {
        if (!_running)
            return 0;

        static uint64_t last_avail_time;
        uint32_t owner;
        if (!mutex_try_enter(&_usb_mutex, &owner))
        {
            if (owner == get_core_num())
                return 0; // would deadlock otherwise
            mutex_enter_blocking(&_usb_mutex);
        }
        int i = 0;
        if (tud_cdc_connected())
        {
            for (int i = 0; i < size;)
            {
                int n = size - i;
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
                    if (!tud_cdc_connected() || (!tud_cdc_write_available() && time_us_64() > last_avail_time + 1000000 /* 1 second */))
                        break;
                }
            }
        }
        else
        {
            last_avail_time = 0; // reset our timeout
        }
        mutex_exit(&_usb_mutex);
        return i;
    }

    int read()
    {
        if (!_running)
            return -1;
        uint32_t owner;
        if (!mutex_try_enter(&_usb_mutex, &owner))
        {
            if (owner == get_core_num())
                return -1; // would deadlock otherwise
            mutex_enter_blocking(&_usb_mutex);
        }
        if (tud_cdc_connected() && tud_cdc_available())
        {
            int ch = tud_cdc_read_char();
            mutex_exit(&_usb_mutex);
            return ch;
        }
        mutex_exit(&_usb_mutex);
        return -1;
    }

    int available(void)
    {
        if (!_running)
            return 0;

        uint32_t owner;
        if (!mutex_try_enter(&_usb_mutex, &owner))
        {
            if (owner == get_core_num())
                return 0; // would deadlock otherwise
            mutex_enter_blocking(&_usb_mutex);
        }
        auto ret = tud_cdc_available();
        mutex_exit(&_usb_mutex);
        return ret;
    }

    int peek(void)
    {
        if (!_running)
            return 0;
        uint8_t c;
        uint32_t owner;
        if (!mutex_try_enter(&_usb_mutex, &owner))
        {
            if (owner == get_core_num())
                return -1; // would deadlock otherwise
            mutex_enter_blocking(&_usb_mutex);
        }

        // auto ret = tud_cdc_peek(0, &c) ? (int)c : -1;
        // auto ret = tud_cdc_peek(&c) ? (int)c : -1;
        auto ret = tud_cdc_n_peek(0, &c) ? (int)c : -1; // SDK 140
        
        mutex_exit(&_usb_mutex);
        return ret;
    }

    void flush(void)
    {
        if (!_running)
            return;
        uint32_t owner;
        if (!mutex_try_enter(&_usb_mutex, &owner))
        {
            if (owner == get_core_num())
                return; // would deadlock otherwise
            mutex_enter_blocking(&_usb_mutex);
        }
        tud_cdc_write_flush();
        mutex_exit(&_usb_mutex);
    }

    operator bool()
    {
        if (!_running)
            return false;

        uint32_t owner;
        if (!mutex_try_enter(&_usb_mutex, &owner))
        {
            if (owner == get_core_num())
                return -1; // would deadlock otherwise
            mutex_enter_blocking(&_usb_mutex);
        }
        tud_task();
        auto ret = tud_cdc_connected();
        mutex_exit(&_usb_mutex);
        return ret;
    }

    static void SerialUSB_irq() { tud_task(); }

    static int64_t SerialUSB_timer_task(__unused alarm_id_t id, __unused void *user_data)
    {
        irq_set_pending(PICO_STDIO_USB_LOW_PRIORITY_IRQ);
        return PICO_STDIO_USB_TASK_INTERVAL_US;
    }

    using Print::write;
};

#endif
#endif // _SERIAL_USB_H_