/*
 * Tone for the Raspberry Pi Pico RP2040
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Arduino.h>
#include <hardware/gpio.h>
#include <pico/time.h>
#undef min
#undef max
#include <map>

#include "PIO.h"
#include "tone.pio.h"
PIOProgram toneProgram(&tone_program);

#define TONE_DBG //printf

typedef struct
{
    pin_size_t pin;
    PIO pio;
    int sm;
} Tone;

static std::map<pin_size_t, Tone *> toneMap;

void tone(uint8_t pin, unsigned int frequency, unsigned long duration)
{
    TONE_DBG("TONE: tone(%d, %d, %d)\n", pin, frequency, duration);
    if (!frequency)
    {
        noTone(pin);
        return;
    }
    int us = 1000000 / frequency / 2;
    if (us < 5)
    {
        us = 5;
    }
    // Even phases run forever, odd phases end after count...so ensure its odd
    int phases = duration ? (duration * 1000 / us) | 1 : 2;
    auto entry = toneMap.find(pin);
    if (entry != toneMap.end())
    {
        noTone(pin);
    }
    TONE_DBG("TONE: phaseUS=%d, phaseCnt=%d\n", us, phases);
    auto newTone = new Tone();
    newTone->pin = pin;
    pinMode(pin, OUTPUT);
    int off;
    if (!toneProgram.prepare(&newTone->pio, &newTone->sm, &off))
    {
        TONE_DBG("TONE: Unable to start, out of PIO resources\n");
        delete newTone; // ERROR, no free slots
        return;
    }
    tone_program_init(newTone->pio, newTone->sm, off, pin);
    pio_sm_set_enabled(newTone->pio, newTone->sm, false);
    pio_sm_put_blocking(newTone->pio, newTone->sm, us * (clock_get_hz(clk_sys) / 1000000));
    pio_sm_exec(newTone->pio, newTone->sm, pio_encode_pull(false, false));
    pio_sm_exec(newTone->pio, newTone->sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(newTone->pio, newTone->sm, true);
    pio_sm_put_blocking(newTone->pio, newTone->sm, phases);
    TONE_DBG("TONE: Began on pio=%p, sm=%d, off=%d\n\n", newTone->pio, newTone->sm, off);
    toneMap.insert({pin, newTone});
}

void noTone(uint8_t pin)
{
    TONE_DBG("NOTONE: noTone(%d)\n", pin);
    auto entry = toneMap.find(pin);
    if (entry != toneMap.end())
    {
        TONE_DBG("NOTONE: Disabling PIO tone generator pio=%p, sm=%d\n", entry->second->pio, entry->second->sm);
        pio_sm_set_enabled(entry->second->pio, entry->second->sm, false);
        pio_sm_unclaim(entry->second->pio, entry->second->sm);
        toneMap.erase(entry);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
}