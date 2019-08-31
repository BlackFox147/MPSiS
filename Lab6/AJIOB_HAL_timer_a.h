#pragma once

#define ACLK_BASE_PERIOD      32768
#define REQUIRED_TIMER_FREQ   1.0

void AJIOB_HAL_timer_a_init();

void AJIOB_HAL_timer_a_start();
void AJIOB_HAL_timer_a_stop();
