#include "tusb.h"
#include "DAP.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"

static void __time_critical_func(dap_main)(void)
{
    DAP_Setup();
    tusb_init();
    while (true)
        tud_task();
}

void __time_critical_func(dap_init)(void)
{
    multicore_launch_core1(dap_main);
}