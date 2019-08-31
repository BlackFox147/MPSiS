#include <msp430.h> 

/*
 * main.c
 */

char lowPowerMode = 0;  // 0 - отключен, 1 - выключен
char lowPowerModeOn = 0;
volatile char lowVoltageMode = 0; // 0 - отключен, 1 - выключен
volatile int i;

void increaseVcoreLevel (unsigned int level) {
    PMMCTL0_H = PMMPW_H;    // открыть PMM регистры дл€ доступа
    SVSMHCTL = SVSHE | SVSHRVL0 * level | SVMHE | SVSMHRRL0 * level;    // управление SVM/SVS на входе
    SVSMLCTL = SVSLE | SVMLE | SVSMLRRL0 * level;   // установка SVM в новый уровень

    while (!(PMMIFG & SVSMLDLYIFG));    // ожидание пока установитс€ SVM

    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);  // очистка ранее установленных флагов
    PMMCTL0_L = PMMCOREV0 * level;  // установка Vcore в новый уровень

    if ((PMMIFG & SVMLIFG)) // ожидание пока будет достигнут новый уровень
        while (!(PMMIFG & SVMLVLRIFG));

    SVSMLCTL = SVSLE | SVSLRVL0 * level | SVMLE | SVSMLRRL0 * level;    // управление SVM/SVS на выходе
    PMMCTL0_H = 0;  // закрыть PMM регистры дл€ доступа
}

void decreaseVcoreLevel (unsigned int level) {
    PMMCTL0_H = PMMPW_H;    // открыть PMM регистры дл€ доступа
    SVSMLCTL = SVSLE | SVSLRVL0 * level | SVMLE | SVSMLRRL0 * level;    // управление SVM/SVS на входе

    while (!(PMMIFG & SVSMLDLYIFG));    // ожидание пока установитс€ SVM

    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);  // очистка ранее установленных флагов
    PMMCTL0_L = PMMCOREV0 * level;  // установка Vcore в новый уровень
    PMMCTL0_H = 0;  // открыть PMM регистры дл€ доступа
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void) {

    //P1IE |= BIT7;
        for(i=0;i<2500;i++);
        //P1IE &= ~BIT7;

        if(!(P1IN & BIT7)){
            P1IFG &= ~BIT7; // сброс флага прерывани€

            if(lowVoltageMode) {
                P1OUT &= ~BIT2; // выключение светодиода 5
                increaseVcoreLevel(2);

                UCSCTL2 = 99;
                UCSCTL3 = SELREF__REFOCLK;          //источник REFOCLK

                P1OUT |= BIT1;  // выключение светодиода 4
            }
            else {
                P1OUT &= ~BIT1; // выключение светодиода 4

                decreaseVcoreLevel(1);

                UCSCTL2 = 49;
                UCSCTL3 = SELREF__XT1CLK;          //источник XT1CLK
                P1OUT |= BIT2;  // выключение светодиода 5
            }
            lowVoltageMode ^= BIT0;
        }

        else{
            P1IFG = 0;
        }
}

#pragma vector=TIMER2_A1_VECTOR
__interrupt void Timer_A(void){
    P1OUT ^= BIT5;
    TA2CTL &= ~TAIFG;
}

#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void) {

    //P2IE |= BIT2;
    for(i=0;i<2500;i++);
    //P2IE &= ~BIT2;

    if(!(P2IN & BIT2)){

        P2IFG &= ~BIT2; // сброс флага прерывани€

        P1OUT ^= BIT0;
        P8OUT ^= BIT2;

        if(lowPowerMode) {
            //_BIC_SR_IRQ(SCG1 + SCG0 + CPUOFF); // выход в LPM3
            LPM3_EXIT;
        }
        else {
            lowPowerModeOn = 1;
        }
        lowPowerMode ^= BIT0;

    }
    else{
        P2IFG = 0;
    }
}

void timer_init()
{
    // timer config

    TA2CTL = TASSEL__ACLK +ID__8 + MC__CONTINUOUS + TACLR; // Use the SMCLK to clock the counter, SMCLK/8, count up mode 8tick/s
    TA2CTL |= TAIE;
    TA2CTL &= ~TAIFG;
}


int main(void) {
    volatile int i = 0;
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    P1DIR |= BIT0 | BIT1 | BIT2 | BIT5; // настройка светодиодов на выход
    P8DIR |= BIT2;
    P7DIR |= BIT7;
    P7SEL |= BIT7;

    P1OUT |= BIT0 | BIT1 | BIT5;    // включение светодиода LED1, LED4, LED8
    P1OUT &= ~BIT2; // выключение светодиода LED4
    P8OUT &= ~BIT2; // выключение светодиода LED3


    P1DIR &= ~BIT7;
    P2DIR &= ~BIT2;

    P1REN |= BIT7;  //разрешение подт€гивающего резистора
    P2REN |= BIT2;

    P1OUT |= BIT7;  //настройка подт€гивающего резистора
    P2OUT |= BIT2;

    P1IES |= BIT7;  //прерывание по переходу из 1 в 0(нажатие кнопки)
    P1IFG &= ~BIT7; //обнуление флага прерывани€ кнопки
    P1IE |= BIT7;   //разрешение прерывани€

    P2IES |= BIT2;  //настройка прерывани€ дл€ второй кнопки
    P2IFG &= ~BIT2;
    P2IE |= BIT2;

    //increaseVcoreLevel(1);
    increaseVcoreLevel(2);

    UCSCTL4 = SELA__DCOCLK + SELS__DCOCLK + SELM__DCOCLK;
    UCSCTL1 = DCORSEL_2;    // выбираем диапазон частот

    UCSCTL2 = 99;
    UCSCTL2 |= FLLD__1;

    UCSCTL3 = SELREF__XT1CLK ;          //источник XT1CLK
    UCSCTL3 |= FLLREFDIV__1;

    timer_init();
    //SFRIE1 |= WDTIE;    //разрешение прерывани€ таймера

    _enable_interrupt();    //включение маскированных прерываний

    while(1)
    {
        if(lowPowerModeOn == 1)
        {
            lowPowerModeOn = 0;
            LPM3;
        }
    }

    return 0;
}
