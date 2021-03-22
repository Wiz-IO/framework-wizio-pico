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

#include "wizio.h"

bool DBG_IS_OPEN = false;
char DBG_BUFFER[DBG_BUFFER_SIZE];

unsigned int strhash(char *src)
{
    unsigned int h = 0, v, i;
    if (src)
    {
        for (h = 0, i = 0; i < strlen(src); i++)
        {
            h = 5527 * h + 7 * src[i];
            v = h & 0x0000ffff;
            h ^= v * v;
        }
    }
    return h;
}

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "hardware/structs/systick.h"
#define SysTick_VAL_CURRENT_Pos 0                                       /*!< SysTick VAL: CURRENT Position */
#define SysTick_VAL_CURRENT_Msk (0xFFFFFFUL << SysTick_VAL_CURRENT_Pos) /*!< SysTick VAL: CURRENT Mask */
#define TICKSGN ((SysTick_VAL_CURRENT_Msk + 1) >> 1)

int SysTick_Config(uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFFUL)
        return -1;               /* Reload value impossible */
    systick_hw->rvr = ticks - 1; /* Set reload */
    systick_hw->cvr = 0;         /* Counter Value */
    systick_hw->csr = 5;         /* Enable  */
    return 0;
}

/*
void waittick(uint32_t step)
{
    static uint32_t tick0;
    if (!step)
    {
        tick0 = systick_hw->cvr;
        return;
    }
    tick0 -= step;
    while ((tick0 - systick_hw->cvr) & TICKSGN);
}
*/