#ifndef __SCCB_H__
#define __SCCB_H__

#include <fx2regs.h>

char sccb_read(char addr);
void sccb_write(char addr, char reg);
void sccb_modify(char addr, char set, char unset);

/* registers */
#define SCCB_COM3 0x0C
#define SCCB_COM7 0x12
#define SCCB_CLKRC 0x11

/* bits */

// COM3
#define bmSCALE_ENABLE bmBIT3

// COM7
#define bmQVGA bmBIT4
#define bmRGB bmBIT2
#define bmCOLORBAR bmBIT1

// CLKRC
#define bmNO_PRESCALE bmBIT6

#endif
