/*
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

    Base: https://github.com/raspberrypi/micropython/blob/pico/ports/rp2/machine_pwm.c
    Edit: Georgi Angelov  
 */

#ifndef __PWM_CLASS_H__
#define __PWM_CLASS_H__

#ifdef __cplusplus

#include "hardware/structs/pwm.h"
#include "hardware/pwm.h"

class PWMClass
{
private:
    uint8_t slice, channel, gpio;

public:
    PWMClass(uint8_t pin, uint32_t freq = 1000)
    {
        gpio = pin;
        slice = pwm_gpio_to_slice_num(gpio);
        channel = pwm_gpio_to_channel(gpio);
        gpio_set_function(gpio, GPIO_FUNC_PWM);
        setFreq(freq);
    }

    uint32_t getFreq()
    {
        uint32_t source_hz = clock_get_hz(clk_sys);
        uint32_t slice_hz = 16 * source_hz / pwm_hw->slice[slice].div;
        uint32_t div16 = pwm_hw->slice[slice].div;
        uint32_t top = pwm_hw->slice[slice].top;
        uint32_t pwm_freq = 16 * source_hz / div16 / top;
        return pwm_freq;
    }

    bool setFreq(uint32_t freq)
    {
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
        pwm_hw->slice[slice].div = div16_top;
        pwm_hw->slice[slice].top = top;
        return true;
    }

    bool setDuty(uint16_t duty, bool ns = false)
    {
        uint32_t cc;
        uint32_t source_hz = clock_get_hz(clk_sys);
        uint32_t top = pwm_hw->slice[slice].top;
        if (ns)
        {
            uint32_t slice_hz = 16 * source_hz / pwm_hw->slice[slice].div;
            cc = (uint64_t)duty * slice_hz / 1000000000ULL;
            if (cc > 65535)
                return false;
        }
        else
        {
            cc = duty * (top + 1) / 65535;
        }
        pwm_set_chan_level(slice, channel, cc);
        pwm_set_enabled(slice, true);
        return true;
    }

    uint16_t getDuty(bool ns = false)
    {
        uint32_t top = pwm_hw->slice[slice].top;
        uint32_t cc = pwm_hw->slice[slice].cc;
        uint32_t source_hz = clock_get_hz(clk_sys);
        uint32_t slice_hz = 16 * source_hz / pwm_hw->slice[slice].div;
        if (ns)
        {
            cc = (cc >> (channel ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB)) & 0xffff;
            return ((uint64_t)cc * 1000000000ULL / slice_hz);
        }
        else
        {
            cc = (cc >> (channel ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB)) & 0xffff;
            return (cc * 65535 / (top + 1));
        }
    }

    void begin(uint16_t duty, bool ns = false) { setDuty(duty, ns); }

    void end() { pwm_set_enabled(slice, false); }
};

#endif // __cplusplus
#endif // CLASS_H