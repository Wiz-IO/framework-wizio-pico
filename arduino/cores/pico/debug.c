////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2021 Georgi Angelov ver 1.0
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

#include "debug.h"

static int dbg_write_r(struct _reent *r, _PTR ctx, const char *buf, int len)
{
    if (len)
    {
        uart_write_blocking(stdout->_cookie, (const uint8_t *)buf, len);
    }
    return len;
}

static uart_inst_t *log_uart = 0;

void dbg_retarget(uart_inst_t *u)
{
    extern void __sinit(struct _reent * s);
    __sinit(_impure_ptr);

    stdout->_cookie = u;
    stdout->_file = STDOUT_FILENO;
    stdout->_flags = __SWID | __SWR | __SNBF;
    stdout->_write = dbg_write_r; // only write
    setvbuf(stdout, NULL, _IONBF, 0);

    //printf("[SYS] PRINTF DEBUG\n");

    log_uart = u;
}
