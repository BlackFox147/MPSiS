#include <msp430.h>

int is_button_on = 0;
int current_button_state = 0;
int led_state = 0;
int current_led = 0;


// LED1 - 1.0
// LED2 - 8.1
// LED3 - 8.2
// S1 - 1.7


//PxIN Ц чтение данных с вывода;
//PxOUT Ц установка значени€ выхода;
//PxDIR Ц выбор направлени€: 0 Ц вход, 1 Ц выход;
//PxREN Ц разрешение подт€гивающего резистора;
//PxSEL Ц выбор функции вывода: 0 Ц I/O, 1 Ц перифери€;
//PxIES Ц выбор направлени€ перепада дл€ генерации запроса на прерывание: 0 Ц по фронту, 1 Ц по спаду;
//PxIE Ц разрешение прерывани€;
//PxIFG Ц флаг прерывани€.

void timer_init();
void button_init();

void main(void) {
        WDTCTL = WDTPW + WDTHOLD;    // отключаем сторожевой таймер

        P1DIR |= BIT0;                          // P1.0 output  LED1
        P8DIR |= BIT1;                          // P8.1 output  LED2
        P8DIR |= BIT2;                          // P8.1 output  LED2


        //P1DIR &= ~BIT7;                         //



        //P1IES |= BIT7;
        //P1IFG &= ~BIT7;

        //P1IE |= BIT7;

        //timer_init();
        button_init();

        //TA2CCTL1 = CCIE; // Enable counter interrupt on counter compare register 0
        TA2CCTL1 &= ~BIT8;
        TA2CTL = TASSEL__SMCLK +ID__2 + MC__CONTINOUS; // Use the SMCLK to clock the counter, SMCLK/8, count up mode 8tick/s

        //TASSEL__SMCLK - выбор источника тактировани€ - 10 SMCLK
        //ID__2 - 01b  /2 - входной делитель
        //MC__CONTINOUS - выбор режима - 10b непрерывный: вверх к 0FFFFh

        //__bis_SR_register(LPM0_bits + GIE);

       while(1);
}


#pragma vector=TIMER2_A1_VECTOR
__interrupt void Timer_A (void){

    if(current_button_state && is_button_on)
    {
        switch(current_led)                     //выбираем, какой диод зажигать
          {
          case 0:

              P1OUT |= BIT0;
              current_led++;
            break;
          case 1:

            P8OUT |= BIT1;
              current_led++;
            break;
          case 2:

                P8OUT |= BIT2;
              current_led++;
          default:
            break;
          }
        if(current_led >= 3)
        {
            current_button_state = 0;
        }
    }

    if(current_button_state && !is_button_on){      //гасим все диоды
        P1OUT &= ~BIT0;
        P8OUT &= ~BIT1;
        P8OUT &= ~BIT2;

        current_button_state = 0;
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void port1_interrupt ()
{
    P1IE &= ~BIT7;                  //disable

    current_button_state = 1;

    led_state = (is_button_on != 1);

    is_button_on = is_button_on ? 0 : 1;

    if(is_button_on){
        current_led = 0;
    }

    P1IE |= BIT7;               //enable
}

void timer_init()
{
  /* Select compare mode */
  //RESET_BITS(TA1CCTL0, BIT8);
    TA2CCTL1 &= ~BIT8;
  //RESET_BITS(TA2CCTL1, BIT8);
  //RESET_BITS(TA2CCTL2, BIT8);

  /* TASSEL = ACLK */
    TA2CTL |= BIT8;
    TA2CTL &=~ BIT9;

  //SET_BITS(TA2CTL, BIT8);
  //RESET_BITS(TA2CTL, BIT9);

  /* ID = /1 */

    TA2CTL &=~ BIT6;
    TA2CTL &=~ BIT7;

  //RESET_BITS(TA2CTL, BIT6);
  //RESET_BITS(TA2CTL, BIT7);

  /* MC = 10b */
    TA2CTL &=~ BIT4;
    TA2CTL |= BIT5;
  //RESET_BITS(TA2CTL, BIT4);
  //SET_BITS(TA2CTL, BIT5);
}

void button_init()
{
    //RESET_BITS(P1DIR, BIT7);
    P1DIR &=~ BIT7;

    //SET_BITS(P1REN, BIT7);
    //SET_BITS(P1OUT, BIT7);

    P1REN |= BIT7;
    P1OUT |= BIT7;

    //SET_BITS(P1IES, BIT7);
    P1IES |= BIT7;
    P1IE |= BIT7;
}
