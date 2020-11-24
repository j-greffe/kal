#ifndef _KAL_DBG_H_
#define _KAL_DBG_H_

#include <stdint.h>
#include <stdbool.h>
#include "hal_gpio.h"

#define ASSERT(_c) kal_dbg_assert((_c), __FILE__, __LINE__);

void kal_dbg_open(gpio_t led);
void kal_dbg_assert(bool condition, char* file, uint32_t line);

#endif // _KAL_DBG_H_
