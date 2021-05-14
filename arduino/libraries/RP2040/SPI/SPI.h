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

#ifndef __SPI_CLASS_H__
#define __SPI_CLASS_H__

#ifdef __cplusplus

#include "interface.h"
#include "constants.h"
#include "Stream.h"
#include "variant.h"
#include "RingBuffer.h"
#include "hardware/spi.h"

#define DEBUG_SPI
//::printf

typedef enum
{
    SPI_MODE0 = 0, // CPOL : 0  | CPHA : 0
    SPI_MODE1,     // CPOL : 0  | CPHA : 1
    SPI_MODE2,     // CPOL : 1  | CPHA : 0
    SPI_MODE3      // CPOL : 1  | CPHA : 1
} SPIDataMode;

class SPISettings
{
public:
    SPISettings(uint32_t clockFrequency, uint8_t bitOrder, uint8_t dataMode)
    {
        clock = clockFrequency;
        order = bitOrder;
        mode = dataMode;
    }

    SPISettings()
    {
        clock = 1000; // kHz
        order = MSBFIRST;
        mode = SPI_MODE0;
    }

private:
    uint32_t clock;
    uint8_t order;
    uint8_t mode;
    friend class SPIClass;
};

#define SPI_SS(L)                  \
    if (_ss > -1)                  \
    {                              \
        asm volatile("nop\n nop"); \
        gpio_put(_ss, L);          \
        asm volatile("nop\n nop"); \
    }

class SPIClass
{
private:
    spi_inst_t *ctx;
    int _sck, _miso, _mosi, _ss;
    int _clk_polarity;
    int _clk_format;
    int _bit_order;
    int _data_bits;
    int _mode;
    uint32_t _brg_hz;

    void init()
    {
        _brg_hz = 4000000;
        _clk_polarity = 0;          // cpol
        _clk_format = 0;            // cpha
        _bit_order = SPI_LSB_FIRST; // LSBFIRST = 0
        _data_bits = 8;             // 4..16
        _mode = 0;
        _miso = -1;
        _mosi = -1;
        _sck = -1;
        _ss = -1;
    }

public:
    SPIClass(uint8_t spi)
    {
        spi ? ctx = spi1 : spi0; // spi0, has two identical SPI
        init();
    }

    SPIClass(spi_inst_t *port)
    {
        ctx = port; // spi0, has two identical SPI
        init();
    }

    // before begin()
    void setPins(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1)
    {
        _sck = sck;
        _miso = miso;
        _mosi = mosi;
        _ss = ss;
        gpio_set_function(_miso, GPIO_FUNC_SPI);
        gpio_set_function(_mosi, GPIO_FUNC_SPI);
        gpio_set_function(_sck, GPIO_FUNC_SPI);
        if (_ss > -1)
        {
            gpio_init(ss);
            gpio_set_dir(ss, GPIO_OUT);
            gpio_put(ss, 1);
        }
    }

    void begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1)
    {
        spi_init(ctx, _brg_hz); // settings.clock
    }

    void beginTransaction(SPISettings settings)
    {
        setFrequency(settings.clock);
        setDataMode(settings.mode);
        setBitOrder(settings.order);
        spi_set_format(ctx, _data_bits, (spi_cpol_t)_clk_polarity, (spi_cpha_t)_clk_format, (spi_order_t)_bit_order);
    }

    void end()
    {
#if 0
        if (_ss > -1)
            gpio_set_function(_ss, GPIO_FUNC_XIP);
        gpio_set_function(_sck, GPIO_FUNC_XIP);
        gpio_set_function(_miso, GPIO_FUNC_XIP);
        gpio_set_function(_mosi, GPIO_FUNC_XIP);
        _miso = -1;
        _mosi = -1;
        _sck = -1;
        _ss = -1;
#endif
        spi_deinit(ctx);
    }

    uint8_t transfer(uint8_t data)
    {
        uint8_t rx = 0;
        SPI_SS(0);
        spi_write_read_blocking(ctx, &data, &rx, 1); // Returns: Number of bytes written/read
        SPI_SS(1);
        return rx; // Returns the received data
    }

    uint16_t transfer16(uint16_t data)
    {
        union
        {
            uint16_t val;
            struct
            {
                uint8_t lsb;
                uint8_t msb;
            };
        } t;
        t.val = data;
        if (_bit_order == SPI_LSB_FIRST)
        {
            t.lsb = transfer(t.lsb);
            t.msb = transfer(t.msb);
        }
        else
        {
            t.msb = transfer(t.msb);
            t.lsb = transfer(t.lsb);
        }
        return t.val;
    }

    int transfer(uint8_t *buf, size_t count)
    {
        SPI_SS(0);
        int res = spi_write_read_blocking(ctx, buf, buf, count);
        SPI_SS(1);
        return res;
    }

    void write(uint8_t data)
    {
        SPI_SS(0);
        spi_write_blocking(ctx, &data, 1);
        SPI_SS(1);
    }

    void write(uint8_t *buf, size_t count)
    {
        SPI_SS(0);
        spi_write_blocking(ctx, buf, count);
        SPI_SS(1);
    }

    void write16(uint16_t data)
    {
        SPI_SS(0);
        spi_write16_blocking(ctx, &data, 1);
        SPI_SS(1);
    }

    void write(uint16_t *buf, size_t count)
    {
        uint8_t backup = _data_bits;
        spi_set_format(ctx, 16, (spi_cpol_t)_clk_polarity, (spi_cpha_t)_clk_format, (spi_order_t)_bit_order);

        SPI_SS(0);
        spi_write16_blocking(ctx, buf, count);
        SPI_SS(1);

        _data_bits = backup;
        spi_set_format(ctx, _data_bits, (spi_cpol_t)_clk_polarity, (spi_cpha_t)_clk_format, (spi_order_t)_bit_order);
    }

    void repeat(uint32_t data, size_t len)
    {
        SPI_SS(0);
        while (len)
        {
            if (spi_is_writable(ctx))
            {
                ((spi_hw_t *)ctx)->dr = data;
                len--;
            }
            else
            {
                tight_loop_contents();
            }
        }
        while (spi_is_readable(ctx))
            (void)spi_get_hw(ctx)->dr;
        while (spi_get_hw(ctx)->sr & SPI_SSPSR_BSY_BITS)
            tight_loop_contents();
        while (spi_is_readable(ctx))
            (void)spi_get_hw(ctx)->dr;
        spi_get_hw(ctx)->icr = SPI_SSPICR_RORIC_BITS;
        SPI_SS(1);
    }

    void repeat(uint16_t data, size_t len)
    {
        uint8_t backup = _data_bits;
        spi_set_format(ctx, 16, (spi_cpol_t)_clk_polarity, (spi_cpha_t)_clk_format, (spi_order_t)_bit_order);
        repeat((uint32_t)data, len);
        _data_bits = backup;
        spi_set_format(ctx, _data_bits, (spi_cpol_t)_clk_polarity, (spi_cpha_t)_clk_format, (spi_order_t)_bit_order);
    }

    void setDataMode(uint8_t mode)
    {
        if (_mode != mode)
        {
            _mode = mode;
            _clk_polarity = mode >> 1;
            _clk_format = mode & 1;
        }
    }

    void setFrequency(uint32_t Hz)
    {
        if (_brg_hz != Hz)
        {
            _brg_hz = Hz;
            //::printf("[SPI] %lu\n", Hz);
            spi_set_baudrate(ctx, Hz);
        }
    }

    void setBitOrder(uint8_t order)
    {
        if (_bit_order != order)
        {
            _bit_order = order;
        }
    }

    uint32_t getClockDivider() { return 0; }
    void setClockDivider(uint8_t div) {}
    void endTransaction(void) {}
    void usingInterrupt(int interruptNumber) {}
    void notUsingInterrupt(int interruptNumber) {}
    void attachInterrupt(){};
    void detachInterrupt(){};
};

extern SPIClass SPI;

#endif // __cplusplus
#endif //__SPI_CLASS_H__