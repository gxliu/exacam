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
#define SCCB_COM14 0x3E
#define SCCB_COM15 0x40
#define SCCB_COM17 0x42
#define SCCB_SCALING_XSC 0x70
#define SCCB_SCALING_YSC 0x71
#define SCCB_SCALING_PCLK_DIV 0x73
#define SCCB_COM10 0x15
#define SCCB_COM2 0x09
#define SCCB_PSHFT 0x1B
#define SCCB_HSTART 0x17
#define SCCB_HSTOP 0x18
#define SCCB_VSTART 0x19
#define SCCB_VSTOP 0x1A
#define SCCB_HREF 0x32
#define SCCB_VREF 0x03

/* bits */

// COM3
#define bmSCALE_ENABLE bmBIT3

// COM7
#define bmCIF  bmBIT5
#define bmQVGA bmBIT4
#define bmQCIF bmBIT3

#define bmRGB bmBIT2
#define bmCOLORBAR bmBIT1

// CLKRC
#define bmNO_PRESCALE bmBIT6

// SCALING_XSC/YSC
#define bmTEST_PATTERN bmBIT7

// COM14
#define bmDCWSCALE bmBIT4

// SCALING_PCLK_DIV
#define bmDSPSCALE bmBIT3

// COM17
#define bmDSPCOLORBAR bmBIT3

// COM10
#define bmPCLK_GATED bmBIT5
#define bmNEG_VSYNC bmBIT1

// COM2
#define bmPWDN bmBIT4

#endif
