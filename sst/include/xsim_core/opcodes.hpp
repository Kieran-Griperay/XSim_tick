#pragma once

#define ADD     0U  // 00000
#define SUB     1U  // 00001
#define AND     2U  // 00010
#define NOR     3U  // 00011
#define DIV     4U  // 00100
#define MUL     5U  // 00101
#define MOD     6U  // 00110
#define EXP     7U  // 00111

#define LW      8U  // 01000
#define SW      9U  // 01001

#define LIZ     16U // 10000
#define LIS     17U // 10001
#define LUI     18U // 10010

#define BP      20U // 10100
#define BN      21U // 10101
#define BX      22U // 10110
#define BZ      23U // 10111

#define JR      12U // 01100
#define JALR    19U // 10011
#define J       24U // 11000

#define HALT    13U // 01101
#define PUT     14U // 01110