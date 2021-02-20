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

#ifndef __WIRE_H__
#define __WIRE_H__
#ifdef __cplusplus

#include "interface.h"
#include "Stream.h"
#include "variant.h"
#include "RingBuffer.h"
#include "hardware/i2c.h"


class TwoWire : public Stream
{
private:
    int _sda, _scl;
    uint32_t _timeOutMillis;

public:
    TwoWire(i2c_inst_t *contex, uint32_t speed_Hz = 100000)
    {
        ctx = contex;
        _sda = -1;
        _scl = -1;
        transmissionBegun = false;
    }

    ~TwoWire() { end(); }

    bool setPins(int sdaPin, int sclPin, bool pull = true)
    {
        _sda = sdaPin;
        _scl = sclPin;
        gpio_set_function(sdaPin, GPIO_FUNC_I2C);
        gpio_set_function(sclPin, GPIO_FUNC_I2C);
        if (pull)
        {
            gpio_pull_up(sdaPin);
            gpio_pull_up(sclPin);
        }
        return true;
    }

    void setTimeOut(uint32_t timeOutMillis) { _timeOutMillis = timeOutMillis; }

    void setClock(uint32_t speed_Hz) { i2c_set_baudrate(ctx, speed_Hz * 1000); }

    void begin(int sdaPin, int sclPin, uint32_t speed_Hz)
    {
        setPins(sdaPin, sclPin);
        i2c_init(ctx, speed_Hz * 1000);
    }

    void end()
    {
        gpio_set_function(_sda, GPIO_FUNC_XIP);
        gpio_set_function(_scl, GPIO_FUNC_XIP);
        i2c_deinit(ctx);
    }

    void beginTransmission(uint8_t);
    uint8_t endTransmission(bool stopBit);
    uint8_t endTransmission(void);

    uint8_t requestFrom(uint8_t address, size_t size, bool stopBit);
    uint8_t requestFrom(uint8_t address, size_t size);

    size_t write(uint8_t data);
    size_t write(const uint8_t *data, size_t size);

    virtual int available(void) { return rx.available(); }
    virtual int read(void) { return rx.read_char(); }
    virtual int peek(void) { return rx.peek(); }
    virtual void flush(void) {}

    using Print::write;

    void onService(void){};
    void onReceive(void (*)(int)){};
    void onRequest(void (*)(void)){};

private:
    bool transmissionBegun;

    RingBuffer rx, tx;

    uint8_t SlaveAddress;
    i2c_inst_t *ctx;
};

extern TwoWire Wire;
extern TwoWire Wire1;

#endif // cpp
#endif // h