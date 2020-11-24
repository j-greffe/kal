#include <kal_dbg.h>

static gpio_t g_led;

void kal_dbg_open(gpio_t led)
{
    g_led = led;
}

void kal_dbg_assert(bool condition, char* file, uint32_t line)
{
    if (!condition)
    {
        hal_gpio_cfg(g_led, HAL_IO_OUT);
        hal_gpio_set(g_led);
        while (1);
    }
}
