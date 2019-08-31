#include <msp430.h> 


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

int is_button_on = 0;
int current_button_state = 0;
int led_state = 0;
int current_led = 0;

typedef enum ccr_channels_t
{
  ccr_button = 1,
  ccr_shift = 2
} ccr_channels_t;

#define SET_BITS(reg, mask)     \
  reg |= mask

#define RESET_BITS(reg, mask)   \
  reg &= ~mask

void led_init();
void button_init();
void button_interrupt_enable();
void button_interrupt_disable();
void button_interrupt_clear();
void timer_init();
void timer_interrupt_enable(ccr_channels_t channel);
void timer_interrupt_disable(ccr_channels_t channel);
void timer_interrupt_clear(ccr_channels_t channel);
void timer_button_callback();
void timer_shift_callback();

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    timer_init();

    button_init();

    led_init();


    timer_interrupt_enable(ccr_shift);

   __bis_SR_register(LPM0_bits + GIE);

  while(1);

}

#pragma vector=PORT1_VECTOR
__interrupt void port1_interrupt ()
{
    button_interrupt_disable();

    timer_interrupt_enable(ccr_button);
}

void timer_init()
{
  /* Select compare mode */
  RESET_BITS(TA2CCTL1, BIT8);
  RESET_BITS(TA2CCTL2, BIT8);

  /* TASSEL = ACLK
   * 01b ACLK */
  SET_BITS(TA2CTL, BIT8);
  RESET_BITS(TA2CTL, BIT9);

  /* ID = /1
   * 00b  /1
   * входной делитель*/
  RESET_BITS(TA2CTL, BIT6);
  RESET_BITS(TA2CTL, BIT7);

  /* MC = 10b
   * 10b непрерывный: вверх к 0FFFFh
   * выбор режима*/
  RESET_BITS(TA2CTL, BIT4);
  SET_BITS(TA2CTL, BIT5);
}


#pragma vector=TIMER2_A1_VECTOR
__interrupt void timer_a1_interrupt()
{
  int val = TA2IV;

  if (val& BIT1)
  {
    timer_button_callback();
  }
  if (val& BIT2)
  {
    timer_shift_callback();
  }
}

#define max_timer_value (0x10000)
const long one_second_timer = max_timer_value / 2;

/* Multiply second to this value */
float timer_custom_divider[] =
{
  1.,
  0.2,
  0.3,
};

void timer_interrupt_enable(ccr_channels_t channel)
{
  /* Set interrupt value */
  unsigned short new_comparator_value = ((unsigned short)(TA2R + (one_second_timer * timer_custom_divider[channel]))) % max_timer_value;

  switch(channel)
  {
  case ccr_button:
    TA2CCR1 = new_comparator_value;
    break;
  case ccr_shift:
    TA2CCR2 = new_comparator_value;
    break;
  default:
    break;
  }

  /* Enable interrupts */
  switch(channel)
  {
  case ccr_button:
    SET_BITS(TA2CCTL1, BIT4);
    break;
  case ccr_shift:
    SET_BITS(TA2CCTL2, BIT4);
    break;
  default:
    break;
  }

  timer_interrupt_clear(channel);
}

void timer_interrupt_disable(ccr_channels_t channel)
{
  switch(channel)
  {
  case ccr_button:
    RESET_BITS(TA2CCTL1, BIT4);
    break;
  case ccr_shift:
    RESET_BITS(TA2CCTL2, BIT4);
    break;
  default:
    break;
  }
}

void timer_interrupt_clear(ccr_channels_t channel)
{
  switch(channel)
  {
  case ccr_button:
    RESET_BITS(TA2CCTL1, BIT0);
    break;
  case ccr_shift:
    RESET_BITS(TA2CCTL2, BIT0);
    break;
  default:
    break;
  }
}

void button_init()
{
    RESET_BITS(P1DIR, BIT7);

    SET_BITS(P1REN, BIT7);
    SET_BITS(P1OUT, BIT7);

    SET_BITS(P1IES, BIT7);
    button_interrupt_enable();
}

void led_init()
{
  int bit = BIT0;

  SET_BITS(P1DIR, bit);
  RESET_BITS(P1OUT, bit);

  bit = BIT1;
  SET_BITS(P8DIR, bit);
  RESET_BITS(P8OUT, bit);

  bit = BIT2;
  SET_BITS(P8DIR, bit);
  RESET_BITS(P8OUT, bit);
}

void button_interrupt_enable()
{
    SET_BITS(P1IE, BIT7);
    button_interrupt_clear();
}

void button_interrupt_disable()
{
    RESET_BITS(P1IE, BIT7);
}

void button_interrupt_clear()
{
    RESET_BITS(P1IFG, BIT7);
}

void timer_button_callback()
{
    current_button_state = 1;
        led_state = (is_button_on != 1);

    is_button_on = is_button_on ? 0 : 1;

    if(is_button_on){
        current_led = 0;
    }

    timer_interrupt_disable(ccr_button);

    button_interrupt_enable();
}

void timer_shift_callback()
{
    if(current_button_state && is_button_on)
    {
        switch(current_led)
          {
          case 0:
              SET_BITS(P1OUT, BIT0);
              current_led++;
            break;
          case 1:
              SET_BITS(P8OUT, BIT1);
              current_led++;
            break;
          case 2:
              SET_BITS(P8OUT, BIT2);
              current_led++;
          default:
            break;
          }
        if(current_led >= 3)
        {
            current_button_state = 0;
        }
    }

    if(current_button_state && !is_button_on){
        RESET_BITS(P1OUT, BIT0);
        RESET_BITS(P8OUT, BIT1);
        RESET_BITS(P8OUT, BIT2);
        current_button_state = 0;
    }
    timer_interrupt_enable(ccr_shift);
}
