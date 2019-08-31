#include <msp430.h> 

char compareLedMode = 0;  // 0 - отключен, 1 - сравнение между PAD3 и PAD1
volatile int i;

void led_init()
{
    P1DIR |= BIT0 | BIT1 | BIT3; // настройка светодиодов на выход
    P8DIR |= BIT2;

    P1OUT &= ~BIT0;
    P1OUT &= ~BIT1;
    P1OUT &= ~BIT3;
    P8OUT &= ~BIT2;
}

void button_init()
{
    P1DIR &= ~BIT7;
    P1REN |= BIT7;  //разрешение подт€гивающего резистора
    P1OUT |= BIT7;  //настройка подт€гивающего резистора
    P1IES |= BIT7;  //прерывание по переходу из 1 в 0(нажатие кнопки)
    P1IFG &= ~BIT7; //обнуление флага прерывани€ кнопки
    P1IE |= BIT7;   //разрешение прерывани€
}

void adc_init()
{
    //enable potentiometer
    ////set potentiometer output as analog input
    P6SEL |= BIT5;
    ////set potentiometer input (max value) to "1"
    P8DIR |= BIT0;
    P8OUT |= BIT0;

    //select start scanning from 0
    ADC12CTL1 &= ~(ADC12CSTARTADD0 | ADC12CSTARTADD1 | ADC12CSTARTADD2 | ADC12CSTARTADD3);
    //Source select = ADC12SC
    ADC12CTL1 &= ~(ADC12SHS0 | ADC12SHS1);
    //adc clock select (SMCLK)
    ADC12CTL1 |= ADC12SSEL0 | ADC12SSEL1;
    //Pulse mode enable
    ADC12CTL1 |= ADC12SHP;
    //repeated multichannel mode select
    ADC12CTL1 |= ADC12CONSEQ0 | ADC12CONSEQ1;
    //enable auto sampling (but need first SHI rising edge)
    ADC12CTL0 |= ADC12MSC;
    //set adc resolution for 8 bits
    ADC12CTL2 &= ~(BIT5 | BIT4);
    //set sample rate buffer to 50ksps
    ADC12CTL2 &= ~ADC12SR;
    // channel 0 config (select potentiometer as source)
    ADC12MCTL0 |= ADC12INCH_5;
    // channel 1 config (select temp as source)
    ///set channel 1 as last for adc
    ADC12MCTL1 |= ADC12EOS;
    ///select channel source
    ADC12MCTL1 |= ADC12INCH_9;
}

void comparator_init()
{
    //Enable Minus & select PAD3 [CB2]
    CBCTL0_H = CBIPEN + CBIPSEL_2;
    //Enable Plus & select PAD1 [CB0]
    CBCTL0_L = CBIPEN + CBIPSEL_0;
    //Set filter delay to 3600ns
    CBCTL1 |= CBFDLY_3;
    //Enable output filter
    CBCTL1 |= CBF;
    //Select interrupt rising edge
    CBCTL1 &= ~CBIES;
    //Set comparator power mode to ultra low power
    CBCTL1 |= BIT9;
    CBCTL1 &= ~BIT8;
    //set other pads as input pullup
    P6DIR &= ~(BIT0 | BIT2 | BIT4);
    P6OUT |= BIT0 | BIT2 | BIT4;

    //Disable interrupt
    //CBINT &= ~CBIE;
    //CBINT &= ~CBIIE;
    //Disable comparator
    //CBCTL1 &= ~CBON;

    //Enable comparator
    CBCTL1 |= CBON;
    //Enable interrupt (main & inverted)
    CBINT |= CBIE;
    CBINT |= CBIIE;
    //clear
    CBINT &= ~CBIFG;
    CBINT &= ~CBIIFG;
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    for(i=0;i<2500;i++);

    if(!(P1IN & BIT7))
    {
        P1IFG &= ~BIT7;

        if(!compareLedMode)
            {
                P1OUT &= ~BIT1;
                P1OUT &= ~BIT3;

                //Disable interrupt
               CBINT &= ~CBIE;
               CBINT &= ~CBIIE;
               //Disable comparator
               CBCTL1 &= ~CBON;

               //enable ADC
               ADC12CTL0 |= ADC12ON;
               //enable ADC convertion and start calc
               ADC12CTL0 |= ADC12ENC | ADC12SC;
               ///enable channel 1 interrupts
               ADC12IE |= ADC12IE1;
               //clear
               ADC12IFG &= ~ADC12IFG1;
            }
        else
        {
            P1OUT &= ~BIT0;
            P8OUT &= ~BIT2;

            //disable interrupts
            ADC12IE &= ~ADC12IE1;
            //disable ADC
            ADC12CTL0 &= ~ADC12ON;
            //disable ADC convertion and reset start calc bit
            ADC12CTL0 &= ~(ADC12ENC | ADC12SC);

            //Enable comparator
            CBCTL1 |= CBON;
            //Enable interrupt (main & inverted)
            CBINT |= CBIE;
            CBINT |= CBIIE;
            //clear
            CBINT &= ~CBIFG;
            CBINT &= ~CBIIFG;
        }

        compareLedMode ^= BIT0;
    }
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	led_init();
	button_init();

	adc_init();
	comparator_init();

	__bis_SR_register(GIE + LPM0_bits);
	while(1);

	return;
}

#pragma vector=COMP_B_VECTOR
__interrupt void comparator_interrupt()
{
    //value = 1 => (+ > -)
   //value = 0 => (+ < -)
   //+ => PAD1
   //- => PAD3

   if(CBCTL1 & CBOUT)
   {
       P1OUT |= BIT1;
       P1OUT &= ~BIT3;
   }
   else{
       P1OUT &= ~BIT1;
       P1OUT |= BIT3;
   }

   //clear
   CBINT &= ~CBIFG;
   CBINT &= ~CBIIFG;
}

#pragma vector=ADC12_VECTOR
__interrupt void adc_interrupt()
{
    short val_U = ADC12MEM1;
    short val_potent = ADC12MEM0;

    if(val_U > val_potent){
        P1OUT |= BIT0;
        P8OUT &= ~BIT2;
    }
    else{
        P1OUT &= ~BIT0;
        P8OUT |= BIT2;
    }

    ADC12IFG &= ~ADC12IFG1;
}

