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

#include "interface.h"

uint32_t arduino_lock; // interrupts() noInterrupts()

uint32_t f_cpu = 0L; // if is 0, frequency is wrong

bool setCpuFrequency(uint32_t freq_hz)
{
    bool res = set_sys_clock_khz(freq_hz / 1000, true);
    if (res)
        f_cpu = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;
    return res;
}

#ifdef PICO_PRINTF_PICO
bool weak_raw_vprintf(const char *fmt, va_list args)
{
    vprintf(fmt, args);
    return true;
}
#endif
