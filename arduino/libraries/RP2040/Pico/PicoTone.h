/*
    Phil Schatzmann 2021 https://www.pschatzmann.ch/

    https://github.com/pschatzmann/pico-arduino/tree/main/Arduino/
    
*/

#pragma once

#include "PicoTimer.h"

/**
 * @brief We use the TimerAlarmRepeating to generate tones.
 */
class PicoTone
{
public:
    PicoTone() {}

    ~PicoTone() { noTone(); }

    static bool generate_sound_callback(repeating_timer_t *rt)
    {
        PicoTone *pt = (PicoTone *)rt->user_data;
        pt->state = !pt->state; // toggle state
        digitalWrite(pt->pin, pt->state);
        return true;
    }

    void tone(uint8_t pinNumber, unsigned int frequency)
    {
        pin = pinNumber;
        int delay = 1000 / frequency / 2;
        alarm.start(generate_sound_callback, delay, MS, this);
    }

    void noTone() { alarm.stop(); }

    bool operator==(const PicoTone &other) { return other.pin == this->pin; }

    bool operator!=(const PicoTone &other) { return other.pin != this->pin; }

    int pin;
    bool state;

protected:
    TimerAlarmRepeating alarm;
};
