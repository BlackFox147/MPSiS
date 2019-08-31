#include "sw_interrupts.h"

#include "driverlib.h"

#include "device_state.h"

#include <stdbool.h>
#include "AJIOB_regs_help.h"
#include "AJIOB_HAL_timer_a.h"
#include "HAL_Cma3000.h"

pot_state_t pot_state;

void Button_S1_ISR()
{
    __disable_interrupt();

    enter_button();

    //sleep 1/8s
    __delay_cycles(131000);

    __enable_interrupt();
}


void x_output_parse()
{
    Cma3000_readAccel();

    pot_state_t new_pot_state =
        (Cma3000_xAccel >= X_SCROLL_RIGHT_MIN) ? pot_state_right :
        (Cma3000_xAccel <= X_SCROLL_LEFT_MAX) ? pot_state_left :
        pot_state_mid;

    if ((new_pot_state != pot_state) || (new_pot_state == pot_state_mid))
    {
        //state was changed or mid state, skip display updating
        AJIOB_HAL_timer_a_reset();
    }

    pot_state = new_pot_state;
}
