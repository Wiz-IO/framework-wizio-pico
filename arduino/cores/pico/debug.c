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

void stdio_usb_out_chars(const char *buf, int length);
int stdio_usb_in_chars(char *buf, int length);

static int dbg_write_r(struct _reent *r, _PTR ctx, const char *buf, int len)
{
    if (len && stdout->_cookie)
    {
        if ((void *)1 == stdout->_cookie)
            stdio_usb_out_chars(buf, len);
        else
            uart_write_blocking(stdout->_cookie, (const uint8_t *)buf, len);
    }
    return -1;
}

static int dbg_read_r(struct _reent *r, _PTR ctx, char *buf, int len)
{
    int rc;
    if (len && stdin->_cookie)
    {
        if ((void *)1 == stdin->_cookie)
        {
            rc = stdio_usb_in_chars(buf, len);
            return rc < 0 ? -1 : rc;
        }
        else
        {
            uart_read_blocking(stdin->_cookie, (uint8_t *)buf, len);
            return len;
        }
    }
    return -1;
}

void dbg_retarget(void *u)
{
    extern void __sinit(struct _reent * s);
    __sinit(_impure_ptr);

    /* STDOUT */
    stdout->_cookie = u;
    stdout->_flags = __SWID | __SWR | __SNBF;
    stdout->_write = dbg_write_r;
    setvbuf(stdout, NULL, _IONBF, 0);

    /* STDERR */
    stderr->_cookie = u;
    stderr->_write = dbg_write_r;
    stdout->_flags = __SWID | __SWR | __SNBF;
    setvbuf(stderr, NULL, _IONBF, 0);

    /* STDIN */
    stdin->_cookie = u;
    stdin->_read = dbg_read_r;
    stdin->_flags = __SWID | __SRD | __SNBF;
    setvbuf(stdin, NULL, _IONBF, 0);
}
