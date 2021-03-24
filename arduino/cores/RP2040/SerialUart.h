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

#ifndef _UART_H_
#define _UART_H_

#ifdef __cplusplus

#include "HardwareSerial.h"
#include <RingBuffer.h>
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "debug.h"

typedef struct tag_UART_CONTEXT
{
    void *cUart;
    void (*rx_handler)(void);
} UART_CONTEXT, *pUART_CONTEXT;

extern UART_CONTEXT UARTINFO[2]; // variant.cpp

static void u0_rx_handler(void);
static void u1_rx_handler(void);

class Uart : public HardwareSerial
{
private:
    uart_inst_t *u;
    pUART_CONTEXT ctx;
    RingBuffer rx_ring; // SERIAL_BUFFER_SIZE = 256
    uint32_t _brg;
    int UART_IRQ;
    int TX_PIN, RX_PIN;

public:
    Uart(uart_inst_t *uart)
    {
        u = uart;
        _brg = 0;
        TX_PIN = 0;
        RX_PIN = 1;
        UART_IRQ = UART0_IRQ;
        if (u == uart0)
        {
            ctx = &UARTINFO[0];
            memset(ctx, 0, sizeof(UART_CONTEXT));
            ctx->rx_handler = u0_rx_handler;
        }
        else
        {
            UART_IRQ = UART1_IRQ;
            ctx = &UARTINFO[1];
            memset(ctx, 0, sizeof(UART_CONTEXT));
            ctx->rx_handler = u1_rx_handler;
            TX_PIN = 4;
            RX_PIN = 5;
        }
        ctx->cUart = this;
    }

    ~Uart() { end(); }

    /* before begin() */
    void pins(int tx, int rx)
    {
        gpio_set_function(tx, GPIO_FUNC_UART);
        gpio_set_function(rx, GPIO_FUNC_UART);
        TX_PIN = tx;
        RX_PIN = rx;
    }

    void begin(unsigned long baud, uint8_t data_bits, uint8_t stop_bits, uint8_t parity, bool retarget = false)
    {
        end();
        pins(TX_PIN, RX_PIN);
        uart_init(u, baud);
        _brg = uart_set_baudrate(u, baud);
        uart_set_hw_flow(u, false, false);
        uart_set_format(u, data_bits, stop_bits, (uart_parity_t)parity);
        uart_set_fifo_enabled(u, false);
        irq_set_exclusive_handler(UART_IRQ, ctx->rx_handler);
        irq_set_enabled(UART_IRQ, true);
        uart_set_irq_enables(u, true, false);
        if (retarget)
        {
            stdio_drv.write_r = u_write_r;
            stdio_drv.read_r = u_read_r;
            stdio_drv.ctx = this;
            dbg_retarget(&stdio_drv);
        }
    }

    void begin(unsigned long baud, bool retarget = false)
    {
        begin(baud, 8, 1, UART_PARITY_NONE, retarget);
    }

    void begin(unsigned long baud, uint8_t config, bool retarget = false)
    {
        begin(baud, config >> 8 & 0xF /*data*/, config & 4 /*stop*/ & 0xF, config & 0xF /*parity*/, retarget);
    }

    void end(void)
    {
        irq_set_enabled(UART_IRQ, false);
        uart_deinit(u);
        rx_ring.clear();
    }

    inline size_t write(uint8_t c)
    {
        uart_write_blocking(u, (const uint8_t *)&c, 1);
        return 1;
    }
    size_t write(const uint8_t *buf, size_t size)
    {
        size_t c = size;
        while (c--)
            uart_putc_raw(u, *buf++);
        return size;
    }
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }

    int read()
    {
        if (!rx_ring.available())
            return -1;
        ENTER_CRITICAL();
        uint8_t byte = rx_ring.read_char();
        EXIT_CRITICAL();
        return byte;
    }
    int read(uint8_t *buf, size_t size)
    {
        int cnt = 0;
        while (available() > 0 && size)
        {
            *buf++ = read();
            size--;
            cnt++;
        }
        return cnt ? cnt : -1;
    }
    int read(char *buf, size_t size) { return read((uint8_t *)buf, size); }

    int available(void) { return rx_ring.available(); }
    int peek(void) { return rx_ring.peek(); }
    void flush(void) { uart_tx_wait_blocking(u); }
    int setSpeed(int brg) { return _brg = uart_set_baudrate(u, brg); }
    int getSpeed() { return _brg; }
    operator bool() { return true; }
    using Print::write;

    // PRIVATE HANDLER
    void isr_save()
    {
        while (uart_is_readable(u))
        {
            if (rx_ring.availableForStore())
            {
                char c = uart_getc(u);
                ENTER_CRITICAL();
                rx_ring.store_char(c);
                EXIT_CRITICAL();
            }
            else
                break;
        }
    }

    // STDIO
    static int u_write_r(struct _reent *r, _PTR p, const char *buf, int len)
    {
        drv_t *d = (drv_t *)p;
        Uart *THIS = (Uart *)d->ctx;
        return THIS->write(buf, len);
    }

    static int u_read_r(struct _reent *r, _PTR p, char *buf, int len)
    {
        drv_t *d = (drv_t *)p;
        Uart *THIS = (Uart *)d->ctx;
        return THIS->read(buf, len);
    }
};

static void u_rx_handler(Uart *ctx) { ctx->isr_save(); }
static void u0_rx_handler(void) { u_rx_handler((Uart *)UARTINFO[0].cUart); }
static void u1_rx_handler(void) { u_rx_handler((Uart *)UARTINFO[1].cUart); }

extern Uart Serial;
extern Uart Serial1;

#endif
#endif // _UART_H_