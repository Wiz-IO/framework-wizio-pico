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

#include <stdio.h>
#include "hardware/rtc.h"

// Executed before main ( crt0.S )
void system_init(void)
{
    // need to link boot2
    void add_bootloader(void);
    add_bootloader();

#ifdef USE_LOCK
    extern void add_lock(void);
    add_lock();
#endif
    extern void __sinit(struct _reent *);
    __sinit(_impure_ptr); 
    stdout->_cookie = NULL;
    stderr->_cookie = NULL;
    stdin->_cookie = NULL;

    // build_flags = -D PICO_STDIO_SEMIHOSTING
    void semihosting_init(void);
    semihosting_init();

    // build_flags = -D PICO_STDIO_UART
    extern void dbg_uart_init(void);
    dbg_uart_init();

    // build_flags = -D PICO_STDIO_USB
    extern void dbg_usb_init(void);
    dbg_usb_init();

    extern void add_syscalls(void);
    add_syscalls();
}
