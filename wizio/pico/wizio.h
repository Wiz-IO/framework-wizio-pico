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

#ifndef _WIZIO_H
#define _WIZIO_H
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
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <reent.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "pico/config.h"
#include "pico/stdlib.h"
#include "pico/divider.h"
#include "pico/bootrom.h"
#include "pico/mutex.h"
#include "pico/multicore.h"
#include "hardware/flash.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/rtc.h"
#include "hardware/divider.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"
#include "hardware/structs/systick.h"

#define PRE_INIT_FUNC(F) static __attribute__((section(".preinit_array"))) void (*__##F)(void) = F
#define INLINE inline __attribute__((always_inline))

#define ENTER_CRITICAL() uint32_t _prim = save_and_disable_interrupts()
#define EXIT_CRITICAL() restore_interrupts(_prim)

#ifdef USE_FREERTOS

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#define MUTEX_PTYPE (SemaphoreHandle_t)
#define MUTEX_INIT(pM) pM = xSemaphoreCreateMutex()
#define MUTEX_LOCK(pM) xSemaphoreTake(MUTEX_PTYPE pM, portMAX_DELAY)
#define MUTEX_UNLOCK(pM) xSemaphoreGive(MUTEX_PTYPE pM)

#else // pico-sdk

#define MUTEX_PTYPE (mutex_t *)
#define MUTEX_INIT(pM)                           \
    pM = MUTEX_PTYPE calloc(1, sizeof(mutex_t)); \
    mutex_init(pM)
#define MUTEX_LOCK(pM) mutex_enter_blocking(MUTEX_PTYPE pM)
#define MUTEX_UNLOCK(pM) mutex_exit(MUTEX_PTYPE pM)

#endif

    unsigned int micros(void);
    unsigned int millis(void);
    unsigned int seconds(void);

    unsigned int strhash(char *src);

    int SysTick_Config(uint32_t ticks);

/* 
    Critical Debug
*/

#define DBG_UART PICO_DEFAULT_UART_INSTANCE
#define DBG_BUFFER_SIZE 256
    extern char DBG_BUFFER[DBG_BUFFER_SIZE];

    extern bool DBG_IS_OPEN;
#define DBG(FRM, ...) /* Serial must be open */                                         \
    if (DBG_IS_OPEN)                                                                    \
    {                                                                                   \
        uint32_t _prim = save_and_disable_interrupts();                                 \
        sprintf(DBG_BUFFER, FRM, ##__VA_ARGS__);                                        \
        uart_write_blocking(DBG_UART, (const uint8_t *)DBG_BUFFER, strlen(DBG_BUFFER)); \
        restore_interrupts(_prim);                                                      \
    }

#define DBG_INIT()                                         \
    {                                                      \
        gpio_set_function(0, 2);                           \
        gpio_set_function(1, 2);                           \
        uart_init(DBG_UART, 115200);                       \
        uart_set_hw_flow(DBG_UART, false, false);          \
        uart_set_format(DBG_UART, 8, 1, UART_PARITY_NONE); \
        uart_set_fifo_enabled(DBG_UART, false);            \
        DBG_IS_OPEN = true;                                \
        memset(DBG_BUFFER, 0, DBG_BUFFER_SIZE);            \
        DBG("[SYS] DEBUG MODE\n");                         \
    }

#ifdef __cplusplus
}
#endif
#endif //_WIZIO_H