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

static int u_write_r(struct _reent *r, _PTR ctx, const char *buf, int len);
static int u_read_r(struct _reent *r, _PTR ctx, char *buf, int len);

class Uart : public HardwareSerial
{
private:
    uart_inst_t *u;
    pUART_CONTEXT ctx;
    RingBuffer rx_ring; // [1024]
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
            TX_PIN = 4; // TODO
            RX_PIN = 5; // TODO
        }
        ctx->cUart = this;
    }

    ~Uart()
    {
        end();
    }

    void pins(int tx, int rx)
    {
        gpio_set_function(tx, GPIO_FUNC_UART);
        gpio_set_function(rx, GPIO_FUNC_UART);
    }

    void begin(unsigned long brg, unsigned int data_bits, unsigned int stop_bits, unsigned int parity, bool retarget = false)
    {
        end();
        pins(TX_PIN, RX_PIN);
        uart_init(u, brg);
        _brg = uart_set_baudrate(u, brg);
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

    void begin(unsigned long brg, bool retarget = false)
    {
        begin(brg, 8, 1, UART_PARITY_NONE, retarget);
    }

    void end(void)
    {
        irq_set_enabled(UART_IRQ, false);
        uart_deinit(u);
        rx_ring.clear();
    }

    size_t write(uint8_t c)
    {
        uart_write_blocking(u, (const uint8_t *)&c, 1);
        return 1;
    }

    int read()
    {
        if (!rx_ring.available())
            return 0;
        __disable_irq();
        uint8_t byte = rx_ring.read_char();
        __enable_irq();
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
        return cnt;
    }
    int read(char *buf, size_t size) { return read((uint8_t *)buf, size); }

    int available(void) { return rx_ring.available(); }

    int peek(void) { return rx_ring.peek(); }

    void flush(void) {}

    int setSpeed(int brg) { return _brg = uart_set_baudrate(u, brg); }

    int getSpeed() { return _brg; }

    operator bool() { return true; }

    using Print::write;

    void isr_save()
    {
        __disable_irq();
        while (uart_is_readable(u))
        {
            if (rx_ring.availableForStore())
                rx_ring.store_char(uart_getc(u));
            else
                break;
        }
        __enable_irq();
    }

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