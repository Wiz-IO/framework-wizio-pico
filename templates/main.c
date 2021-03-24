/*  
    Created : 01.01.2021
    Author  : https://github.com/Wiz-IO/wizio-pico

upload_port   = PICO-DRIVE:/ 
monitor_port  = COM* ... /dev/tty*
monitor_speed = 115200

 */

#include <stdio.h>
#include "pico/stdlib.h"

#define LED PICO_DEFAULT_LED_PIN

int main(void)
{  
    stdio_init_all();
    printf("\n\nHello World\n");
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    while (true)
    {
        gpio_put(LED, 1);
        sleep_ms(100);
        gpio_put(LED, 0);
        sleep_ms(100);
    }
}
