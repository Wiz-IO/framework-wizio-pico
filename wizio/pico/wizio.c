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

#ifdef USE_DEBUG
#include "dbg.h"
bool DBG_IS_OPEN = false;
char DBG_BUFFER[DBG_BUFFER_SIZE];
#endif

unsigned int strhash(const void *p)
{
    unsigned int h = 0, v, i;
    char * src = (char*)p;
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

int SysTick_Config(uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFFUL)
        return -1;               /* Reload value impossible */
    systick_hw->rvr = ticks - 1; /* Set reload */
    systick_hw->cvr = 0;         /* Counter Value */
    systick_hw->csr = 5;         /* Enable  */
    return 0;
}
