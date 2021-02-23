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

#ifndef INTERFACE_H_
#define INTERFACE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <reent.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>

#include "pico/stdlib.h"
#include "pico/divider.h"   
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/rtc.h"
#include "hardware/divider.h"
#include "hardware/clocks.h"

#include <debug.h>

#define INLINE inline __attribute__((always_inline))

    __attribute__((always_inline)) static __inline void __disable_irq(void)
    {
        __asm volatile("cpsid i"
                       :
                       :
                       : "memory");
    }

    __attribute__((always_inline)) static __inline void __enable_irq(void)
    {
        __asm volatile("cpsie i"
                       :
                       :
                       : "memory");
    }

    static inline void interrupts(void) { __enable_irq(); }
    static inline void noInterrupts(void) { __disable_irq(); }

    void attachInterruptEx(uint8_t pin, void (*cb)(uint32_t pin_event), int mode); // enum gpio_irq_level

    static inline unsigned int millis(void)
    {
        //return time_us_64() / 1000;
        return div_u64u64(time_us_64(), 1000);
    }

    static inline unsigned int micros(void)
    {
        return time_us_32();
    }

    static inline unsigned int seconds(void)
    {
        return millis() / 1000;
    }

    static inline void delay(unsigned int ms)
    {
        sleep_ms(ms);
    }

    static inline void delayMicroseconds(unsigned int us)
    {
        sleep_us(us);
    };

    static inline void delaySeconds(unsigned int s)
    {
        sleep_ms(s * 1000);
    };

    char *utoa(unsigned int value, char *buffer, int radix);
    INLINE char *ltoa(long value, char *result, int base) { return utoa(value, result, base); }
    INLINE char *ultoa(unsigned long value, char *result, int base) { return utoa(value, result, base); }

    uint32_t clockCyclesPerMicrosecond(void);
    uint32_t clockCyclesToMicroseconds(uint32_t a);
    uint32_t microsecondsToClockCycles(uint32_t a);

    void analogInit(uint8_t adc_channel);
    int analogRead(uint8_t adc_channel);
    void analogWrite(uint8_t pin, int val);
    float temperatureRead(void);

    void initVariant(void) __attribute__((weak));

#ifndef SERIAL_BUFFER_SIZE
#define SERIAL_BUFFER_SIZE 1024
#endif

#define digitalPinToPort(p)
#define digitalPinToBitMask(p)
#define digitalPinToClkid(p)
#define digitalPinSPIAvailiable(p)
#define digitalPinToSPIDevice(p)
#define digitalPinToSPIClockId(p)

#ifdef __cplusplus
}

#endif

#endif /* INTERFACE_H_ */
