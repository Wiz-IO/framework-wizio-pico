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
#include <hardware/gpio.h>

#define PINS_COUNT 30

bool pinGetDir(uint8_t pin)
{
    return gpio_get_dir(pin);
}

void pinMode(pin_size_t pinNumber, PinMode pinMode)
{
    if (pinNumber >= PINS_COUNT) //
        return;
    gpio_init(pinNumber);
    if (pinMode & (OUTPUT | OUTPUT_LO | OUTPUT_HI))
    {
        gpio_set_dir(pinNumber, GPIO_OUT);
        if (pinMode & OUTPUT_LO)
            gpio_put(pinNumber, 0);
        else
            gpio_put(pinNumber, 1);
    }
    else if (pinMode & (INPUT | INPUT_PULLUP | INPUT_PULLDOWN))
    {
        gpio_set_dir(pinNumber, GPIO_IN);
        if (pinMode & INPUT_PULLUP)
            gpio_pull_up(pinNumber);
        else
            gpio_pull_down(pinNumber);
    }
}

void digitalWrite(pin_size_t pinNumber, PinStatus status)
{
    // if (pin >= PINS_COUNT) return;
    gpio_put(pinNumber, status);
}

PinStatus digitalRead(pin_size_t pinNumber)
{
    // if (pin >= PINS_COUNT) return 0;
    return gpio_get(pinNumber);
}

////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    int mode;
    voidFuncPtr cb;
    //voidFuncPtrParam cb_ex;
} pin_callback_t;

static pin_callback_t pin_handlers[PINS_COUNT];

static void irq_gpio_callback(uint gpio, uint32_t events) // one per core
{
    if (gpio < PINS_COUNT)
    {
        pin_callback_t *p = &pin_handlers[gpio];
        if (p->cb)
            p->cb();
    }
}

void attachInterrupt(pin_size_t pin, voidFuncPtr callback, PinStatus mode) // enum gpio_irq_level
{
    if (callback && pin < PINS_COUNT)
    {
        pin_handlers[pin].cb = callback;
        gpio_set_irq_enabled_with_callback(pin, mode, true, &irq_gpio_callback);
    }
}

void detachInterrupt(pin_size_t pin)
{
    if (pin < PINS_COUNT)
    {
        pin_handlers[pin].cb = NULL;
        gpio_set_irq_enabled_with_callback(pin, pin_handlers[pin].mode, false, NULL);
    }
}