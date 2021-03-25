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
#include "hardware/irq.h"
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

#endif // USE_FREERTOS

    unsigned int strhash(const void *p);

    int SysTick_Config(uint32_t ticks);

#ifdef __cplusplus
}
#endif
#endif //_WIZIO_H