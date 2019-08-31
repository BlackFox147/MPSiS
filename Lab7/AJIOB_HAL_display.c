#include "AJIOB_HAL_display.h"

#include "driverlib.h"
#include "HAL_Dogs102x6.h"
#include "Animation.h"

char input_to_char[] = {
    '0',            //inputs_digit0
    '1',            //inputs_digit1
    '+',            //inputs_operationPlus
    '-',            //inputs_operationMinus
    '*',            //inputs_operationMul
    '%',            //inputs_operationMod
    'R',            //inputs_memoRead
    'W',            //inputs_memoWrite
    '=',            //inputs_nextCalc
};

int8_t start_pos = 0;

// internal prototypes
#define CONST_COUNT(A) (sizeof(A)/sizeof(A[0]))

uint8_t digit_to_symbol(uint8_t digit);

//definitions

void AJIOB_HAL_display_init()
{
    Dogs102x6_init();
    Dogs102x6_backlightInit();
    Dogs102x6_setBacklight(5);

    const int frame_count = CONST_COUNT(Animation_array);

    Dogs102x6_imageDraw(Animation_array[frame_count - 1], 0, 0);
    __delay_cycles(DISPLAY_LOGO_SLEEP_TICKS);

    for (int i = frame_count - 1; i >= 0; i--)
    {
        Dogs102x6_imageDraw(Animation_array[i], 0, 0);
        __delay_cycles(DISPLAY_LOGO_ANIMATION_TICKS / CONST_COUNT(Animation_array));
    }

    Dogs102x6_imageDraw(Animation_array[0], 0, 0);
    __delay_cycles(DISPLAY_LOGO_SLEEP_TICKS);
}

void AJIOB_HAL_display_print_input_mode(inputs_t mode)
{
    char new_char = input_to_char[mode];
    Dogs102x6_charDraw(DISPLAY_FONT_Y_INPUT, 0, new_char, DOGS102x6_DRAW_NORMAL);
}

void AJIOB_HAL_display_print_answer(long val, bool sign)
{
    int8_t symbols = 6;

    for (int8_t i = 0; i < symbols-1; i++)
    {
        int8_t start_pos = symbols - i - 1;
        uint16_t digit = digit_to_symbol(val & 0x1);

        Dogs102x6_charDraw(DISPLAY_FONT_Y_ANSWER, start_pos * DISPLAY_FONT_X_STEP,
            digit, DOGS102x6_DRAW_NORMAL);

        val >>= 1;
    }

    Dogs102x6_charDraw(DISPLAY_FONT_Y_ANSWER, 0,
                       digit_to_symbol(sign), DOGS102x6_DRAW_NORMAL);

}

void AJIOB_HAL_display_print_append_nums(inputs_t input)
{
    char new_char = input_to_char[input];
    Dogs102x6_charDraw(DISPLAY_FONT_Y_NUMS, start_pos * DISPLAY_FONT_X_STEP, new_char, DOGS102x6_DRAW_NORMAL);

    start_pos++;
}

void AJIOB_HAL_display_print_reset_num_pos()
{
    start_pos = 0;
}

uint8_t digit_to_symbol(uint8_t digit)
{
    return input_to_char[inputs_digit0 + digit];
}
