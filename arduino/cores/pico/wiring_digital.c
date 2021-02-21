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

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin >= PINS_COUNT)
        return;
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
        if (mode & INPUT_PULLUP)
            gpio_pull_up(pin);
        else
            gpio_pull_down(pin);
    }
}

INLINE void digitalWrite(uint8_t pin, uint8_t val)
{
    // if (pin >= PINS_COUNT) return;
    gpio_put(pin, val);
}

INLINE int digitalRead(uint8_t pin)
{
    // if (pin >= PINS_COUNT) return 0;
    return gpio_get(pin);
}

////////////////////////////////////////////////////////////////////////////////////////

typedef void (*pin_cb_t)(void);
typedef void (*pin_cb_ex_t)(uint32_t event);

typedef struct
{
    int mode;
    pin_cb_t cb;
    pin_cb_ex_t cb_ex;
} pin_callback_t;

static pin_callback_t pin_handlers[PINS_COUNT];

static void irq_gpio_callback(uint gpio, uint32_t events) // one per core
{
    if (gpio < PINS_COUNT)
    {
        pin_callback_t *p = &pin_handlers[gpio];
        if (p->cb)
            p->cb();
        if (p->cb_ex)
            p->cb_ex(events);
    }
}

void attachInterruptEx(uint8_t pin, void (*cb)(uint32_t event), int mode) // enum gpio_irq_level
{
    if (cb && pin < PINS_COUNT)
    {
        pin_handlers[pin].cb_ex = cb;
        gpio_set_irq_enabled_with_callback(pin, mode, true, &irq_gpio_callback);
    }
}

void attachInterrupt(uint8_t pin, void (*cb)(void), int mode) // enum gpio_irq_level
{
    if (cb && pin < PINS_COUNT)
    {
        pin_handlers[pin].cb = cb;
        gpio_set_irq_enabled_with_callback(pin, mode, true, &irq_gpio_callback);
    }
}

void detachInterrupt(uint8_t pin)
{
    if (pin < PINS_COUNT)
    {
        pin_handlers[pin].cb = NULL;
        pin_handlers[pin].cb_ex = NULL;
        gpio_set_irq_enabled_with_callback(pin, pin_handlers[pin].mode, false, NULL);
    }
}
