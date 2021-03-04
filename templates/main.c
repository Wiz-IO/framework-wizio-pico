/**
    PlatformIO - wizio-pico
    https://github.com/Wiz-IO

    Created : 01.01.2021
    Author  : 


ADD TO platformio.ini (  Wiki: https://github.com/Wiz-IO/wizio-pico/wiki  )

upload_port   = select PICO-DRIVE:/ or select HARD-DRIVE:/ to save the UF2 file ( example C:/ )
monitor_port  = COM* ... /dev/tty*
monitor_speed = 115200
;lib_deps = 
;build_flags = 

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
