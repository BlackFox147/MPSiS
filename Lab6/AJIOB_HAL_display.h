#pragma once

#include <stdint.h>

#define DISPLAY_LINES_TO_SHOW 2
#define DISPLAY_ROW_MAX_X_PLUS 100
#define DISPLAY_PLUS_MAX 255

#define DISPLAY_LOGO_SLEEP_TICKS 25000000

void AJIOB_HAL_display_init();
void AJIOB_HAL_display_print_value(int16_t val);
