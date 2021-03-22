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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "debug.h"

drv_t stdio_drv;

void dbg_retarget(void *p)
{
    /* STDOUT */
    stdout->_cookie = p;
    stdout->_write = ((drv_t *)p)->write_r;
    stdout->_flags = __SWID | __SWR | __SNBF;
    setvbuf(stdout, NULL, _IONBF, 0);

    /* STDERR */
    stderr->_cookie = p;
    stderr->_write = ((drv_t *)p)->write_r;
    stderr->_flags = __SWID | __SWR | __SNBF;
    setvbuf(stderr, NULL, _IONBF, 0);

    /* STDIN */
    stdin->_cookie = p;
    stdin->_read = ((drv_t *)p)->read_r;
    stdin->_flags = __SWID | __SRD | __SNBF;
    setvbuf(stdin, NULL, _IONBF, 0);
}

#ifdef PICO_STDIO_UART

static int dbg_uart_write_r(struct _reent *r, _PTR p, const char *buf, int len)
{
    uart_write_blocking(((drv_t *)p)->ctx, (uint8_t *)buf, len);
    return len;
}

static int dbg_uart_read_r(struct _reent *r, _PTR p, char *buf, int len)
{
    if (irq_is_enabled(UART0_IRQ) || irq_is_enabled(UART1_IRQ))
        return -1; // is used form Serial
    uart_read_blocking(((drv_t *)p)->ctx, (uint8_t *)buf, len);
    return len;
}

void dbg_uart_init(void)
{
    uart_deinit(DBG_UART);
    gpio_set_function(PICO_DEFAULT_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PICO_DEFAULT_UART_RX_PIN, GPIO_FUNC_UART);
    uart_init(DBG_UART, 115200);
    uart_set_hw_flow(DBG_UART, false, false);
    uart_set_format(DBG_UART, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(DBG_UART, false);
    stdio_drv.ctx = DBG_UART;
    stdio_drv.write_r = dbg_uart_write_r;
    stdio_drv.read_r = dbg_uart_read_r;
    dbg_retarget(&stdio_drv);
}
#else
void dbg_uart_init(void){}
#endif // PICO_STDIO_UART

#ifdef PICO_STDIO_SEMIHOSTING
static void __attribute__((naked)) semihosting_putc(char c)
{
    __asm(
        "mov r1, r0\n"
        "mov r0, #3\n"
        "bkpt 0xab\n"
        "bx lr\n");
}

static int semihosting_write_r(struct _reent *r, _PTR p, const char *buf, int len)
{
    for (uint i = 0; i < len; i++)
        semihosting_putc(buf[i]);
    return len;
}

void semihosting_init(void)
{
    stdio_drv.ctx = semihosting_init; // NOT NULL
    stdio_drv.write_r = semihosting_write_r;
    dbg_retarget(&stdio_drv);
}
#else
void semihosting_init(void){}
#endif // PICO_STDIO_SEMIHOSTING