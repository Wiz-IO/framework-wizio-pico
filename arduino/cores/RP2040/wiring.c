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

typedef struct
{
    int events;
    voidFuncPtr callback;
    voidFuncPtrParam callback_ex;
    uint8_t pwm_slice;
    uint8_t pwm_channel;
    uint32_t pwm_frequency;
} pin_t;

static pin_t pins[PINS_COUNT];

inline bool pinGetDir(uint8_t pin)
{
    return gpio_get_dir(pin);
}

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

static void irq_gpio_callback(uint gpio, uint32_t events) // one per core
{
    if (gpio < PINS_COUNT)
    {
        pin_t *p = &pins[gpio];
        if (p->callback)
            p->callback();
        if (p->callback_ex)
            p->callback_ex((void *)events);
    }
}

void attachInterruptEx(uint8_t pin, voidFuncPtrParam cb, int mode) // enum gpio_irq_level
{
    if (cb && pin < PINS_COUNT)
    {
        pins[pin].callback_ex = cb;
        gpio_set_irq_enabled_with_callback(pin, mode, true, &irq_gpio_callback);
    }
}

void attachInterrupt(uint8_t pin, void (*cb)(void), int mode) // enum gpio_irq_level
{
    if (cb && pin < PINS_COUNT)
    {
        pins[pin].callback = cb;
        gpio_set_irq_enabled_with_callback(pin, mode, true, &irq_gpio_callback);
    }
}

void detachInterrupt(uint8_t pin)
{
    if (pin < PINS_COUNT)
    {
        pins[pin].callback = NULL;
        pins[pin].callback_ex = NULL;
        gpio_set_irq_enabled_with_callback(pin, pins[pin].events, false, NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////////////

bool analogSetFrequency(uint8_t pin, uint32_t freq)
{
    pins[pin].pwm_frequency = 0;
    pins[pin].pwm_slice = pwm_gpio_to_slice_num(pin);
    pins[pin].pwm_channel = pwm_gpio_to_channel(pin);
    uint32_t source_hz = clock_get_hz(clk_sys);
// Set the frequency, making "top" as large as possible for maximum resolution.
// Maximum "top" is set at 65534 to be able to achieve 100% duty with 65535.
#define TOP_MAX 65534
    uint32_t div16_top = 16 * source_hz / freq;
    uint32_t top = 1;
    for (;;)
    {
        // Try a few small prime factors to get close to the desired frequency.
        if (div16_top >= 16 * 5 && div16_top % 5 == 0 && top * 5 <= TOP_MAX)
        {
            div16_top /= 5;
            top *= 5;
        }
        else if (div16_top >= 16 * 3 && div16_top % 3 == 0 && top * 3 <= TOP_MAX)
        {
            div16_top /= 3;
            top *= 3;
        }
        else if (div16_top >= 16 * 2 && top * 2 <= TOP_MAX)
        {
            div16_top /= 2;
            top *= 2;
        }
        else
        {
            break;
        }
    }
    if (div16_top < 16)
    {
        return false; // freq too large
    }
    else if (div16_top >= 256 * 16)
    {
        return false; // freq too small
    }
    pwm_hw->slice[pins[pin].pwm_slice].div = div16_top;
    pwm_hw->slice[pins[pin].pwm_slice].top = top;
    pins[pin].pwm_frequency = freq;
    return true;
}

void analogWrite(uint8_t pin, int duty)
{
    if (pin < PINS_COUNT)
    {
        if (0 == pins[pin].pwm_frequency)
        {
            if (false == analogSetFrequency(pin, 1000))
                return;
            gpio_set_function(pin, GPIO_FUNC_PWM);
        }
        uint32_t cc;
        uint32_t top = pwm_hw->slice[pins[pin].pwm_slice].top;
        cc = duty * (top + 1) / 65535;
        pwm_set_chan_level(pins[pin].pwm_slice, pins[pin].pwm_channel, cc);
        pwm_set_enabled(pins[pin].pwm_slice, true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////

void analogInit(uint8_t adc_channel)
{
    static int adc_once = 0;
    if (!adc_once)
    {
        adc_once = 1;
        adc_init();
    }
    adc_channel += 26;                         // GPIO26
    gpio_set_pulls(adc_channel, false, false); // Make sure GPIO is high-impedance, no pullups etc
    if (adc_channel > 29)                      // GPIO29
        adc_set_temp_sensor_enabled(true);
    else
        adc_gpio_init(adc_channel);
}

int analogRead(uint8_t adc_channel)
{
    adc_select_input(adc_channel); // Select ADC_0 (GPIO26), ADC_T is TEMPERATURE
    return adc_read();
}

float temperatureRead(void)
{
    const float t = analogRead(ADC_T) * 3.3f / (1 << 12);
    return 27.0 - (t - 0.706) / 0.001721;
}