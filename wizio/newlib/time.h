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

#ifndef _PLATFORM_TIME_H
#define _PLATFORM_TIME_H
#ifdef __cplusplus
extern "C"
{
#endif
#include_next <time.h>
//#warning TEST <TIME.H>

#include <stdbool.h>
    
#define _POSIX_TIMERS 1
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC (clockid_t)4
#endif
#ifndef CLOCK_BOOTTIME
#define CLOCK_BOOTTIME (clockid_t)4
#endif

    int clock_settime(clockid_t clock_id, const struct timespec *tp);
    int clock_gettime(clockid_t clock_id, struct timespec *tp);
    int clock_getres(clockid_t clock_id, struct timespec *res);

    time_t now(void);
    bool set_now_tm(struct tm *p);
    bool set_now(time_t T);    

    /* ARDUINO LIKE visible from <time.h> */
    unsigned int micros(void);
    unsigned int millis(void);
    unsigned int seconds(void);
    void delay(unsigned int ms);
    void delayMicroseconds(unsigned int us);

#ifdef __cplusplus
}
#endif
#endif /* _PLATFORM_TIME_H */
