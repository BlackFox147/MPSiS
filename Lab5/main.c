#include "types.h"
#include "clk.h"
#include "types.h"
#include "spi_display.h"
#include "spi.h"
#include "Regs_help.h"
/**
 * main.c
 */

#pragma vector=PORT1_VECTOR
__interrupt void port1_interrupt()
{

    //button_interrupt_disable();

    //timer_interrupt_enable();

    //P7OUT |= BIT7;

    volatile int i;
    for(i=0;i<3000;i++);

    if(!(P1IN & BIT7))
    {
        is_need_to_change_mirror = true;
    }

    P1IFG &= ~BIT7;
}

//#pragma vector=PORT2_VECTOR
/*__interrupt void port2_interrupt()
{
  short value = P2IV;
  switch(value)
  {
  case 0x0C:    //P2IFG.5
    //accelerometer interrupt
    accelerometer_interrupt_handle();
    break;
  default:
    break;
  }

  P2IFG = 0;
}
*/

void button_init()
{
    P1DIR &= ~BIT7;
    P1REN |= BIT7;  //разрешение подт€гивающего резистора
    P1OUT |= BIT7;  //настройка подт€гивающего резистора
    P1IES |= BIT7;  //прерывание по переходу из 1 в 0(нажатие кнопки)
    P1IFG &= ~BIT7; //обнуление флага прерывани€ кнопки
    P1IE |= BIT7;   //разрешение прерывани€
}

void long_delay()
{
  __delay_cycles(10000000);
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P7DIR |= BIT7;
	P7OUT &= ~BIT7;

    clk_init();
    button_init();

    spi_init();
    spi_display_init();

    GIE_ENABLE;

    display_init();

    accelerometer_init();

    //LPM0;

    while(1){
        accelerometer_interrupt_handle();
    }
}
