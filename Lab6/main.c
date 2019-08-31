
#include "driverlib.h"
#include "HAL_Cma3000.h"
#include "AJIOB_HAL_display.h"
#include "AJIOB_HAL_timer_a.h"
#include "AJIOB_HAL_file.h"
#include "AJIOB_HAL_buttons.h"
#include "sw_interrupts.h"
#include "HAL_Wheel.h"

#define READY_LED_PORT  GPIO_PORT_P8
#define READY_LED_PIN   GPIO_PIN2

void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  Cma3000_init();
  Wheel_init();

  AJIOB_HAL_display_init();
  AJIOB_HAL_buttons_init();
  AJIOB_HAL_init_file();
  AJIOB_HAL_timer_a_init();

  // board is ready
  GPIO_setAsOutputPin(READY_LED_PORT, READY_LED_PIN);
  GPIO_setOutputHighOnPin(READY_LED_PORT, READY_LED_PIN);

  __enable_interrupt();

  while (1)
  {
    if (AJIOB_HAL_buttons_is_pressed_S2())
    {
      Button_S2_ISR();
    }

    Cma3000_readAccel();
    if (Cma3000_yAccel > 10)
    {
      Y_ISR();
    }

    // skip 1/6 s
    __delay_cycles(4000000);
  }
}
