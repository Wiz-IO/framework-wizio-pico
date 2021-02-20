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

#include <Arduino.h>
#include "Wire.h"

uint8_t TwoWire::requestFrom(uint8_t address, size_t size, bool stopBit)
{
    SlaveAddress = address;
    if (size == 0)
        return 0;
    if (!stopBit)
        return 0;
    rx.clear();
    int res = i2c_read_timeout_us(ctx, SlaveAddress, rx._aucBuffer, size, false, _timeOutMillis);
    if (res < 0)
        size = 0;
    rx._iHead = size;
    return rx.available();
}

uint8_t TwoWire::requestFrom(uint8_t address, size_t size)
{
    return requestFrom(address, size, true);
}

void TwoWire::beginTransmission(uint8_t address)
{
    SlaveAddress = address;
    tx.clear();
    transmissionBegun = true;
}

uint8_t TwoWire::endTransmission(bool stopBit)
{
    if (!stopBit)
        return 1;
    transmissionBegun = false;
    int size = tx.available();
    int res = i2c_write_timeout_us(ctx, SlaveAddress, tx._aucBuffer, size, false, _timeOutMillis);
    return res == size ? 0 : 0; // 0:success or 4:other error
}

uint8_t TwoWire::endTransmission() { return endTransmission(true); }

size_t TwoWire::write(uint8_t ucData)
{
    if (!transmissionBegun || tx.isFull())
        return 0;
    tx.store_char(ucData);
    return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        if (!write(data[i]))
            return i;
    }
    return size;
}

TwoWire Wire(i2c0);
TwoWire Wire1(i2c1);