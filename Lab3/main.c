#include <msp430.h> 

/*
 * main.c
 */

char lowPowerMode = 0;  // 0 - ��������, 1 - ��������
char lowPowerModeOn = 0;
volatile char lowVoltageMode = 0; // 0 - ��������, 1 - ��������
volatile int i;

void increaseVcoreLevel (unsigned int level) {
    PMMCTL0_H = PMMPW_H;    // ������� PMM �������� ��� �������
    SVSMHCTL = SVSHE | SVSHRVL0 * level | SVMHE | SVSMHRRL0 * level;    // ���������� SVM/SVS �� �����
    SVSMLCTL = SVSLE | SVMLE | SVSMLRRL0 * level;   // ��������� SVM � ����� �������

    while (!(PMMIFG & SVSMLDLYIFG));    // �������� ���� ����������� SVM

    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);  // ������� ����� ������������� ������
    PMMCTL0_L = PMMCOREV0 * level;  // ��������� Vcore � ����� �������

    if ((PMMIFG & SVMLIFG)) // �������� ���� ����� ��������� ����� �������
        while (!(PMMIFG & SVMLVLRIFG));

    SVSMLCTL = SVSLE | SVSLRVL0 * level | SVMLE | SVSMLRRL0 * level;    // ���������� SVM/SVS �� ������
    PMMCTL0_H = 0;  // ������� PMM �������� ��� �������
}

void decreaseVcoreLevel (unsigned int level) {
    PMMCTL0_H = PMMPW_H;    // ������� PMM �������� ��� �������
    SVSMLCTL = SVSLE | SVSLRVL0 * level | SVMLE | SVSMLRRL0 * level;    // ���������� SVM/SVS �� �����

    while (!(PMMIFG & SVSMLDLYIFG));    // �������� ���� ����������� SVM

    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);  // ������� ����� ������������� ������
    PMMCTL0_L = PMMCOREV0 * level;  // ��������� Vcore � ����� �������
    PMMCTL0_H = 0;  // ������� PMM �������� ��� �������
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void) {

    //P1IE |= BIT7;
        for(i=0;i<2500;i++);
        //P1IE &= ~BIT7;

        if(!(P1IN & BIT7)){
            P1IFG &= ~BIT7; // ����� ����� ����������

            if(lowVoltageMode) {
                P1OUT &= ~BIT2; // ���������� ���������� 5
                increaseVcoreLevel(2);

                UCSCTL2 = 99;
                UCSCTL3 = SELREF__REFOCLK;          //�������� REFOCLK

                P1OUT |= BIT1;  // ���������� ���������� 4
            }
            else {
                P1OUT &= ~BIT1; // ���������� ���������� 4

                decreaseVcoreLevel(1);

                UCSCTL2 = 49;
                UCSCTL3 = SELREF__XT1CLK;          //�������� XT1CLK
                P1OUT |= BIT2;  // ���������� ���������� 5
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

        P2IFG &= ~BIT2; // ����� ����� ����������

        P1OUT ^= BIT0;
        P8OUT ^= BIT2;

        if(lowPowerMode) {
            //_BIC_SR_IRQ(SCG1 + SCG0 + CPUOFF); // ����� � LPM3
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

    P1DIR |= BIT0 | BIT1 | BIT2 | BIT5; // ��������� ����������� �� �����
    P8DIR |= BIT2;
    P7DIR |= BIT7;
    P7SEL |= BIT7;

    P1OUT |= BIT0 | BIT1 | BIT5;    // ��������� ���������� LED1, LED4, LED8
    P1OUT &= ~BIT2; // ���������� ���������� LED4
    P8OUT &= ~BIT2; // ���������� ���������� LED3


    P1DIR &= ~BIT7;
    P2DIR &= ~BIT2;

    P1REN |= BIT7;  //���������� �������������� ���������
    P2REN |= BIT2;

    P1OUT |= BIT7;  //��������� �������������� ���������
    P2OUT |= BIT2;

    P1IES |= BIT7;  //���������� �� �������� �� 1 � 0(������� ������)
    P1IFG &= ~BIT7; //��������� ����� ���������� ������
    P1IE |= BIT7;   //���������� ����������

    P2IES |= BIT2;  //��������� ���������� ��� ������ ������
    P2IFG &= ~BIT2;
    P2IE |= BIT2;

    //increaseVcoreLevel(1);
    increaseVcoreLevel(2);

    UCSCTL4 = SELA__DCOCLK + SELS__DCOCLK + SELM__DCOCLK;
    UCSCTL1 = DCORSEL_2;    // �������� �������� ������

    UCSCTL2 = 99;
    UCSCTL2 |= FLLD__1;

    UCSCTL3 = SELREF__XT1CLK ;          //�������� XT1CLK
    UCSCTL3 |= FLLREFDIV__1;

    timer_init();
    //SFRIE1 |= WDTIE;    //���������� ���������� �������

    _enable_interrupt();    //��������� ������������� ����������

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
