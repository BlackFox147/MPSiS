#pragma once

#include <msp430.h>

#define BIT10 BITA
#define BIT11 BITB
#define BIT12 BITC
#define BIT13 BITD
#define BIT14 BITE
#define BIT15 BITF

typedef int bool;
#define true 1
#define false 0

typedef unsigned char uint8_t;
#define nullptr 0

typedef unsigned char volatile out_port_type_t;

#define GIE_ENABLE __bis_SR_register(GIE)
