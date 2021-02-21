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
    void *user; // class Uart
    uart_inst_t *UART;
    void (*rx_handler)(void);
    void (*tx_handler)(void);
} UART_CONTEXT, *pUART_CONTEXT;
extern UART_CONTEXT UARTINFO[2];

static void u0_rx_handler(void);
static void u1_rx_handler(void);

class Uart : public HardwareSerial
{
private:
    uart_inst_t *u;
    pUART_CONTEXT ctx;
    RingBuffer rx_ring; // [1024]
    uint32_t _brg;
    int UART_IRQ;

public:
    Uart(uart_inst_t *uart)
    {
        u = uart;
        _brg = 0;
        int TX_PIN = 0, RX_PIN = 1;
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

        ctx->user = this;
        pins(TX_PIN, RX_PIN);
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

    void begin(unsigned long brg, unsigned int data_bits, unsigned int stop_bits, unsigned int parity, bool dbg = false)
    {
        end();
        uart_init(u, brg);
        _brg = uart_set_baudrate(u, brg);
        uart_set_hw_flow(u, false, false);
        uart_set_format(u, data_bits, stop_bits, (uart_parity_t)parity);
        uart_set_fifo_enabled(u, false);

        irq_set_exclusive_handler(UART_IRQ, ctx->rx_handler);
        irq_set_enabled(UART_IRQ, true);
        uart_set_irq_enables(u, true, false);

        if (dbg)
            dbg_retarget(u);
    }

    void begin(unsigned long brg, bool dbg = false)
    {
        begin(brg, 8, 1, UART_PARITY_NONE, dbg);
    }

    void end(void)
    {
        irq_set_enabled(UART_IRQ, false);
        uart_deinit(u);
        rx_ring.clear();
    }

    size_t write(uint8_t c)
    {
        uart_putc_raw(u, c);
        return 1;
    }

    size_t write(uint8_t *buf, size_t size)
    {
        uart_write_blocking(u, buf, size);
        return size;
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

    int available(void)
    {
        return rx_ring.available();
    }

    int peek(void)
    {
        return rx_ring.peek();
    }

    void flush(void) {}

    int setSpeed(int brg)
    {
        return _brg = uart_set_baudrate(u, brg);
    }

    int getSpeed()
    {
        return _brg;
    }

    void save()
    {
        while (uart_is_readable(u))
        {
            if (rx_ring.availableForStore())
                rx_ring.store_char(uart_getc(u));
            else
                break;
        }
    }

    operator bool()
    {
        return true;
    }

    using Print::write;
};

static void u_rx_handler(Uart *ctx)
{
    __disable_irq();
    ctx->save();
    __enable_irq();
}
static void u0_rx_handler(void) { u_rx_handler((Uart *)UARTINFO[0].user); }
static void u1_rx_handler(void) { u_rx_handler((Uart *)UARTINFO[1].user); }

extern Uart Serial;
extern Uart Serial1;

#endif
#endif // _UART_H_