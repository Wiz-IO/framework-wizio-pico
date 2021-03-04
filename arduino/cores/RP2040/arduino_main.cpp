/*
    Created on: 2021
    Author: Georgi Angelov
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
 */

#include <interface.h>

void setup();
void loop();
void initVariant();

static void TaskMain(void *pvParameters)
{
    initVariant();    
    setup();
    for (;;)
    {
        loop();
        yield();
    }
}

int main(void)
{
#ifdef USE_FREERTOS
    xTaskCreate(
        TaskMain,
        "TaskMain",                         /* Text name for the task. */
        PICO_STACK_SIZE / sizeof(uint32_t), /* Stack size in words, not bytes. */
        NULL,                               /* Parameter  */
        1,                                  /* Priority  */
        0);
    vTaskStartScheduler();
    while (1)
        abort();
#endif

    TaskMain(NULL);
}
