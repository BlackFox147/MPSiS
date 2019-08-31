#include "clk.h"
#include "types.h"
#include "AJIOB_regs_help.h"

#define SELM_SHIFT 0
#define SELS_SHIFT 4

#define SEL_BASIC_MASK (BIT0 | BIT1 | BIT2)

#define XT1CLK_VALUE 0
#define DCOCLK_VALUE 3

#define SELM_MASK (SEL_BASIC_MASK << SELM_SHIFT)
#define SELS_MASK (SEL_BASIC_MASK << SELS_SHIFT)

#define SELM_XT1CLK (XT1CLK_VALUE << SELM_SHIFT)
#define SELS_XT1CLK (XT1CLK_VALUE << SELS_SHIFT)

#define SELM_DCOCLK (DCOCLK_VALUE << SELM_SHIFT)
#define SELS_DCOCLK (DCOCLK_VALUE << SELS_SHIFT)

void clk_init()
{
    //DCORSEL sel to 4 interval (need 8 MHz)
    RESET_BITS(UCSCTL1, (BIT4 | BIT5 | BIT6));
    SET_BITS(UCSCTL1, (BIT4 | BIT5));
    //SELREF => XT1CLK
    RESET_BITS(UCSCTL3, (BIT4 | BIT5 | BIT6));
    //FLLREFDIV => /1
    RESET_BITS(UCSCTL3, (BIT2 | BIT1 | BIT0));
    //FLLD => /1
    RESET_BITS(UCSCTL2, (BIT12 | BIT13 | BIT14));
    //FLLN => 199
    RESET_BITS(UCSCTL2, 0x3FF);
    SET_BITS(UCSCTL2, 199);

    //MCLK enable to DCOCLK
    RESET_BITS(UCSCTL4, SELM_MASK);
    SET_BITS(UCSCTL4, SELM_DCOCLK);

    //SMCLK enable to DCOCLK
    RESET_BITS(UCSCTL4, SELS_MASK);
    SET_BITS(UCSCTL4, SELS_DCOCLK);

    //Pin 7.7 as peripheral
    SET_BITS(P7SEL, BIT7);
    SET_BITS(P7DIR, BIT7);
}
