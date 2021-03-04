////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2021 Georgi Angelov
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

#include <errno.h>
#include <sys/times.h> // struct tms
#include "interface.h"

#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif

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

time_t _Time(time_t *timer) // TODO
{
    return now(); // HOST_SERVICE (SVC_TIME);
}

time_t _time(time_t *tod)
{
    time_t t = _Time(NULL);
    if (tod)
        *tod = t;
    return (t);
}

//clock_t _times(struct tms *tp) { return time_us_64(); }
clock_t _times_r(struct _reent *r, struct tms *ptms)
{
#ifdef USE_FREERTOS
    ptms->tms_stime = xTaskGetTickCount() * (portTICK_PERIOD_MS * CLK_TCK / 1000);
#else
    ptms->tms_stime = time_us_64(); /* ???? system time */
#endif
    ptms->tms_cstime = 0; /* system time of children */
    ptms->tms_cutime = 0; /* user time of children */
    ptms->tms_utime = 0;  /* user time */
    struct timeval tv = {0, 0};
    _gettimeofday_r(r, &tv, NULL);
    return (clock_t)tv.tv_sec;
}

int _gettimeofday_r(struct _reent *ignore, struct timeval *tv, void *tz) /* time() */
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

int usleep(useconds_t us) // TODO usleep()
{
#ifdef USE_FREERTOS
    const int us_per_tick = portTICK_PERIOD_MS * 1000;
#else
    const int us_per_tick = 0;      // TODO
#endif
    if (us < us_per_tick)
    {
        sleep_us((uint32_t)us);
    }
    else
    {
#ifdef USE_FREERTOS
        vTaskDelay((us + us_per_tick - 1) / us_per_tick);
#endif
    }
    return 0;
}

unsigned int sleep(unsigned int seconds)
{
    usleep(seconds * 1000000UL);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////

void add_time(void)
{
    datetime_t t = {
        .year = 2021,
        .month = 3,
        .day = 1,
        .dotw = 0, // ?
        .hour = 0,
        .min = 0,
        .sec = 0};
    rtc_init(); // Start the RTC
    rtc_set_datetime(&t);
}