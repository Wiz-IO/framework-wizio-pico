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

// TODO test

#include <sys/times.h> // struct tms
#include <sys/time.h>
#include <reent.h>
#include <errno.h>

#include "hardware/rtc.h"
#include "pico/stdlib.h"

#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif

long timezone = 0;

time_t now(void)
{
    datetime_t t;
    rtc_get_datetime(&t);
    struct tm ti;
    ti.tm_sec = t.sec;          /// pico < 0..59
    ti.tm_min = t.min;          /// pico < 0..59
    ti.tm_hour = t.hour;        /// pico < 0..23
    ti.tm_mday = t.day;         /// pico < 1..28,29,30,31 depending on month
    ti.tm_mon = t.month - 1;    /// pico < 1..12, 1 is January
    ti.tm_year = t.year - 1900; /// pico < 0..4095
    ti.tm_wday = t.dotw;        /// pico < 0..6, 0 is Sunday
    return mktime(&ti);
}

int _gettimeofday(struct timeval *tv, void *tz)
{
    (void)tz;
    if (tv)
    {
        if (rtc_running())
        {
            tv->tv_sec = now();
            tv->tv_usec = 0;
        }
        else
        {
            uint64_t microseconds = time_us_64();
            tv->tv_sec = microseconds / 1000000;
            tv->tv_usec = microseconds % 1000000;
        }
        return 0;
    }
    errno = EINVAL;
    return -1;
}

int _gettimeofday_r(struct _reent *ignore, struct timeval *tv, void *tz)
{
    return _gettimeofday(tv, tz);
}

/* returns the number of clock ticks that have elapsed since an arbitrary point in the past. */
clock_t _times_r(struct _reent *r, struct tms *ptms) /* clock() */
{
#ifdef USE_FREERTOS
    ptms->tms_stime = xTaskGetTickCount() * (portTICK_PERIOD_MS * CLK_TCK / 1000);
#else
    ptms->tms_stime = to_ms_since_boot(get_absolute_time()); /* system time */
#endif
    ptms->tms_cstime = 0; /* system time of children */
    ptms->tms_cutime = 0; /* user time of children */
    ptms->tms_utime = 0;  /* user time */

    struct timeval tv = {0, 0};
    _gettimeofday_r(r, &tv, NULL);
    return (clock_t)tv.tv_sec;
}

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    if (tp == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    struct timeval tv;
    uint64_t monotonic_time_us = 0;
    switch (clock_id)
    {
    case CLOCK_REALTIME:
        _gettimeofday_r(NULL, &tv, NULL);
        tp->tv_sec = tv.tv_sec;
        tp->tv_nsec = tv.tv_usec * 1000L;
        break;
    case CLOCK_MONOTONIC:
        monotonic_time_us = to_us_since_boot(get_absolute_time());
        tp->tv_sec = monotonic_time_us / 1000000LL;
        tp->tv_nsec = (monotonic_time_us % 1000000LL) * 1000L;
        break;
    default:
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int usleep(uint64_t us) // useconds_t
{
#ifndef USE_FREERTOS
    sleep_us((uint64_t)us);
#else
    const int us_per_tick = portTICK_PERIOD_MS * 1000;
    if (us < us_per_tick)
    {
        sleep_us((uint64_t)us);
    }
    else
    {
        vTaskDelay((us + us_per_tick - 1) / us_per_tick);
    }
#endif
    return 0;
}

unsigned int sleep(unsigned int seconds)
{
    usleep(seconds * 1000000UL);
    return 0;
}

/* ARDUINO LIKE visible from <time.h> */

unsigned int micros(void)
{
    return to_us_since_boot(get_absolute_time());
}

unsigned int millis(void)
{
    return to_ms_since_boot(get_absolute_time());
}

unsigned int seconds(void)
{
    return millis() / 1000;
}

__inline void delayMicroseconds(unsigned int us)
{
    sleep_us(us);
};

__inline void delay(unsigned int ms)
{
#ifdef USE_FREERTOS
    vTaskDelay(pdMS_TO_TICKS(ms));
#else
    sleep_ms(ms);
#endif
}

bool set_now_tm(struct tm *p)
{
    datetime_t dt;
    dt.sec = p->tm_sec;   /// pico < 0..59
    dt.min = p->tm_min;   /// pico < 0..59
    dt.hour = p->tm_hour; /// pico < 0..23
    dt.day = p->tm_mday;  /// pico < 1..28,29,30,31 depending on month
    dt.month = p->tm_mon; /// pico < 1..12, 1 is January
    dt.year = p->tm_year; /// pico < 0..4095
    dt.dotw = p->tm_wday; /// pico < 0..6, 0 is Sunday
    return rtc_set_datetime(&dt);
}

bool set_now(time_t t)
{
    return set_now_tm(localtime(&t));
}

void include_time(void)
{
#ifdef ARDUINO
    datetime_t t = {
        .year = 2021,
        .month = 3,
        .day = 25,
        .dotw = 0, // ?
        .hour = 0,
        .min = 0,
        .sec = 0};
    rtc_init(); // Start the RTC
    rtc_set_datetime(&t);
#endif
    //setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    //tzset();
}
