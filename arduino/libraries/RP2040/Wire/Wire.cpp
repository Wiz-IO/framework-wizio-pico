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

#include <Arduino.h>
#include "Wire.h"

uint8_t TwoWire::requestFrom(uint8_t address, size_t size, bool stopBit)
{
    _slave_address = address;
    if (size == 0)
        return 0;
    if (!stopBit)
        return 0;
    rx.clear();
    int res = i2c_read_timeout_us(ctx, _slave_address, rx._aucBuffer, size, 0, _timeout_us);
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
    _slave_address = address;
    tx.clear();
    transmissionBegun = true;
}

uint8_t TwoWire::endTransmission(bool stopBit)
{
    if (!stopBit)
        return 1;
    int res;
    transmissionBegun = false;
    int size = tx.available();
    res = i2c_write_timeout_us(ctx, _slave_address, tx._aucBuffer, size, 1, _timeout_us);
    //res = i2c_write_blocking(ctx, _slave_address, tx._aucBuffer, size, 1);
    return res == size ? 0 : 4;
    // 0:success
    // 1:data too long to fit in transmit buffer
    // 2:received NACK on transmit of address
    // 3:received NACK on transmit of data
    // 4:other error
}

uint8_t TwoWire::endTransmission()
{
    return endTransmission(true);
}

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