
/* Critical Debug -D USE_DEBUG */

#include <stdio.h>
#include <string.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define DBG_BUFFER_SIZE 256
extern char DBG_BUFFER[DBG_BUFFER_SIZE];
extern bool DBG_IS_OPEN;

#define DBG(FRM, ...)                                                                   \
    if (DBG_IS_OPEN)                                                                    \
    {                                                                                   \
        snprintf(DBG_BUFFER, DBG_BUFFER_SIZE, FRM, ##__VA_ARGS__);                      \
        uart_write_blocking(PICO_DEFAULT_UART_INSTANCE, (const uint8_t *)"[DBG] ", 6);                    \
        uart_write_blocking(PICO_DEFAULT_UART_INSTANCE, (const uint8_t *)DBG_BUFFER, strlen(DBG_BUFFER)); \
    }

#define DBG_INIT()                                         \
    {                                                      \
        gpio_set_function(0, 2);                           \
        gpio_set_function(1, 2);                           \
        uart_init(PICO_DEFAULT_UART_INSTANCE, 115200);                       \
        uart_set_hw_flow(PICO_DEFAULT_UART_INSTANCE, false, false);          \
        uart_set_format(PICO_DEFAULT_UART_INSTANCE, 8, 1, UART_PARITY_NONE); \
        uart_set_fifo_enabled(PICO_DEFAULT_UART_INSTANCE, false);            \
        DBG_IS_OPEN = true;                                \
        memset(DBG_BUFFER, 0, DBG_BUFFER_SIZE);            \
        DBG("DEBUG MODE\n");                               \
    }
