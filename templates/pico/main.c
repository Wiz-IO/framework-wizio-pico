/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
    PlatformIO PasberryPi Pico
        Created on: 01.01.2021
        Author: Georgi Angelov  
        https://github.com/Wiz-IO/

Add to platformio.ini

upload_port   = select PICO-DRIVE:/ or select HARD-DRIVE:/ to save the UF2 file ( example C:/ )
monitor_port  = COMx
monitor_speed = 115200

 */

#include <stdio.h>
#include "pico/stdlib.h"

int main(void)
{  
    stdio_init_all();
    printf("\n\nHello World\n");
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true)
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(50);
        gpio_put(LED_PIN, 0);
        sleep_ms(50);
    }
}
