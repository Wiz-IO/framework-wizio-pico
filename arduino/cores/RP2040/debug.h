////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020 Georgi Angelov ver 1.0
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

#ifndef _DBG_H_
#define _DBG_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

    #define DBG_UART uart0

    typedef struct
    {
        void *ctx;
        int (*write_r)(struct _reent *r, _PTR ctx, const char *buf, int len);
        int (*read_r)(struct _reent *r, _PTR ctx, char *buf, int len);
    } drv_t;

    extern drv_t stdio_drv;

    void dbg_retarget(void *p);

#ifdef __cplusplus
}
#endif
#endif