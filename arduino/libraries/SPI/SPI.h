/*
    TODO 

    Author: Georgi Angelov
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA   
 */

#ifndef __SPI_CLASS_H__
#define __SPI_CLASS_H__

#ifdef __cplusplus

#include "interface.h"
#include "constants.h"
#include "Stream.h"
#include "variant.h"
#include "RingBuffer.h"
#include "hardware/spi.h"

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
    uint16_t clock;
    uint8_t order;
    uint8_t mode;
    friend class SPIClass;
};

class SPIClass
{
protected:
    spi_inst_t *ctx; // spi0, spi1

private:
    int _sck, _miso, _mosi, _ss;

public:
    SPIClass(spi_inst_t *port)
    {
        ctx = port;
    }

    void begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss = -1)
    {
        gpio_set_function(miso, GPIO_FUNC_SPI);
        gpio_set_function(mosi, GPIO_FUNC_SPI);
        gpio_set_function(sck, GPIO_FUNC_SPI);
        _miso = miso;
        _mosi = mosi;
        _sck = miso;
        if (ss > -1)
        {
            gpio_init(ss);
            gpio_set_dir(ss, GPIO_OUT);
            gpio_put(ss, 1);
            _ss = ss;
        }
        spi_init(ctx, 100 * 1000); // settings.clock
    }

    void beginTransaction(SPISettings settings)
    {
        setFrequency(settings.clock);
        setDataMode(settings.mode);
        setBitOrder(settings.order);
        //TODO
    }

    void end()
    {
        if (_ss > -1)
            gpio_set_function(_ss, GPIO_FUNC_XIP);
        gpio_set_function(_sck, GPIO_FUNC_XIP);
        gpio_set_function(_miso, GPIO_FUNC_XIP);
        gpio_set_function(_mosi, GPIO_FUNC_XIP);
        _miso = -1;
        _mosi = -1;
        _sck = -1;
        _ss = -1;
        spi_deinit(ctx);
    }

    uint8_t transfer(uint8_t data)
    {
        if (_ss > -1)
        {
            asm volatile("nop \n nop \n nop");
            gpio_put(_ss, 0);
            asm volatile("nop \n nop \n nop");
        }

        uint8_t rd = 0;
        spi_write_read_blocking(ctx, &data, &rd, 1);

        if (_ss > -1)
        {
            asm volatile("nop \n nop \n nop");
            gpio_put(_ss, 1);
            asm volatile("nop \n nop \n nop");
        }
        return rd;
    }

    uint16_t transfer16(uint16_t data)
    {
        return 0;
    }

    void transfer(uint8_t *buf, size_t count)
    {
        spi_write_read_blocking(ctx, buf, buf, count);
    }

    void setBitOrder(uint8_t order)
    {
    }

    void setDataMode(uint8_t mode)
    {
    }

    void setFrequency(uint16_t kHz) { spi_set_baudrate(ctx, kHz * 1000); }
    void setClockDivider(uint8_t div) {}

    void endTransaction(void) {}
    void usingInterrupt(int interruptNumber) {}
    void notUsingInterrupt(int interruptNumber) {}
    void attachInterrupt(){};
    void detachInterrupt(){};
};

extern SPIClass SPI;
extern SPIClass SPI1;

#endif // __cplusplus
#endif //__SPI_CLASS_H__