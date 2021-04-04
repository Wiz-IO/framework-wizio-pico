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
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include <hardware/pwm.h>
#include <hardware/clocks.h>

void analogInit(uint8_t adc_channel)
{
    static int adc_once = 0;
    if (!adc_once)
    {
        adc_once = 1;
        adc_init();
    }
    adc_channel += 26;    // GPIO26
    if (adc_channel > 29) // GPIO29
        adc_set_temp_sensor_enabled(true);
    else
        adc_gpio_init(adc_channel); // Make sure GPIO is high-impedance, no pullups etc
}

int analogRead(uint8_t adc_channel)
{
    adc_select_input(adc_channel); // Select ADC_0 (GPIO26), ADC_T is TEMPERATURE
    return adc_read();
}

float temperatureRead(void)
{
    const float t = analogRead(4) * 3.3f / (1 << 12);
    return 27.0 - (t - 0.706) / 0.001721;
}

static int32_t analogScale = 255;
static uint16_t analogFreq = 1000;
static bool pwmInitted = false;

void analogWrite(pin_size_t pin, int val)
{
    if (!pwmInitted)
    {
        pwm_config c = pwm_get_default_config();
        pwm_config_set_clkdiv(&c, clock_get_hz(clk_sys) / (float)(analogScale * analogFreq));
        pwm_config_set_wrap(&c, analogScale);
        for (int i = 0; i < 30; i++)
        {
            pwm_init(pwm_gpio_to_slice_num(i), &c, true);
        }
        pwmInitted = true;
    }
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(pin, (uint16_t)val);
}