#pragma once

#define PAD_INTERRUPT 1
#define X_SCROLL_RIGHT_MIN    20
#define X_SCROLL_LEFT_MAX     -20

void Button_S1_ISR();
void x_output_parse();

typedef enum pot_state_t
{
    pot_state_left  = -1,
    pot_state_mid   = 0,
    pot_state_right = 1,
} pot_state_t;

extern pot_state_t pot_state;
