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

#include <Arduino.h>

////////////////////////////////////////////////////////////////////////////////////////

INLINE void interrupts(void) { __enable_irq(); };
INLINE void noInterrupts(void) { __disable_irq(); };

////////////////////////////////////////////////////////////////////////////////////////

void pinMode(uint8_t pin, uint8_t mode)
{
    gpio_init(pin);
    if (mode & (OUTPUT | OUTPUT_LO | OUTPUT_HI))
    {
        gpio_set_dir(pin, GPIO_OUT);
        if (mode & OUTPUT_LO)
            gpio_put(pin, 0);
        else
            gpio_put(pin, 1);
    }
    else if (mode & (INPUT | INPUT_PULLUP | INPUT_PULLDOWN))
    {
        gpio_set_dir(pin, GPIO_IN);
        // pull todo
    }
}

INLINE void digitalWrite(uint8_t pin, uint8_t val)
{
    gpio_put(pin, val);
}

INLINE int digitalRead(uint8_t pin)
{
    return gpio_get(pin);
}

////////////////////////////////////////////////////////////////////////////////////////
