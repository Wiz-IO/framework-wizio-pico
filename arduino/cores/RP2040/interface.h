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

#ifndef INTERFACE_H_
#define INTERFACE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <wizio.h>
#include "arduino_debug.h"

typedef void (*voidFuncPtr)(void);
typedef void (*voidFuncPtrParam)(void*);

#define PRE_INIT_FUNC(F) static __attribute__((section(".preinit_array"))) void (*__##F)(void) = F
#define INLINE inline __attribute__((always_inline))

#define ENTER_CRITICAL() uint32_t _prim = save_and_disable_interrupts()
#define EXIT_CRITICAL() restore_interrupts(_prim)

#ifndef SERIAL_BUFFER_SIZE
#define SERIAL_BUFFER_SIZE 256
#endif

    extern uint32_t arduino_lock;
    static INLINE void interrupts(void) { restore_interrupts(arduino_lock); }
    static INLINE void noInterrupts(void) { arduino_lock = save_and_disable_interrupts(); }
    void attachInterruptEx(uint8_t pin, voidFuncPtrParam cb, int mode);
    
    extern uint32_t f_cpu;
    bool setCpuFrequency(uint32_t freq_hz);
#define F_CPU f_cpu
#define clockCyclesPerMicrosecond() (F_CPU / 1000000L)
#define clockCyclesToMicroseconds(a) (((a)*1000L) / (F_CPU / 1000L))
#define microsecondsToClockCycles(a) ((a)*clockCyclesPerMicrosecond())
#define getSysTick() systick_hw->cvr

    extern char *utoa(unsigned int value, char *buffer, int radix);
    static inline char *ltoa(long value, char *result, int base) { return utoa(value, result, base); }
    static inline char *ultoa(unsigned long value, char *result, int base) { return utoa(value, result, base); }

    bool pinGetDir(uint8_t pin);
#define digitalPinToPort(p)
#define digitalPinToBitMask(p)
#define digitalPinToClkid(p)
#define digitalPinSPIAvailiable(p)
#define digitalPinToSPIDevice(p)
#define digitalPinToSPIClockId(p)
    float temperatureRead(void);
    bool analogSetFrequency(uint8_t pin, uint32_t freq);

#define run_core(core1_loop_func) multicore_launch_core1(core1_loop_func)
#define reset_core() multicore_reset_core1()
#define send_message(MSG) multicore_fifo_push_blocking(MSG)
#define send_message_us(MSG) multicore_fifo_push_timeout_us(MSG, TIMEOUT_US)
#define get_message() multicore_fifo_pop_blocking()
#define get_message_us(MSG) multicore_fifo_pop_blocking(TIMEOUT_US, MSG)

    static INLINE void yield(void)
    {
#ifdef USE_FREERTOS
        vTaskDelay(0);
#endif
    }

#ifdef __cplusplus
}

#endif
#endif /* INTERFACE_H_ */
