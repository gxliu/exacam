#ifndef __SCCB_H__
#define __SCCB_H__

#include <fx2regs.h>

char sccb_read(char addr);
void sccb_write(char addr, char reg);
void sccb_modify(char addr, char set, char unset);

/* registers */
#define SCCB_COM1 0x04
#define SCCB_COM3 0x0C
#define SCCB_COM4 0x0D
#define SCCB_COM5 0x0E
#define SCCB_COM6 0x0F
#define SCCB_COM7 0x12
#define SCCB_COM8 0x13
#define SCCB_COM9 0x14
#define SCCB_COM11 0x3B
#define SCCB_COM12 0x3C
#define SCCB_COM13 0x3D
#define SCCB_GFIX 0x69
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
#define SCCB_ABLC1 0xB1
#define SCCB_COM16 0x41
#define SCCB_TSLB 0x3A
#define SCCB_RGB444 0x8C
#define SCCB_GAIN 0x00
#define SCCB_AECH 0x07
#define SCCB_BD50MAX 0xA5
#define SCCB_BD60MAX 0xAB
#define SCCB_AEW 0x24
#define SCCB_AEB 0x25
#define SCCB_VPT 0x26
#define SCCB_HAECC1 0x9F
#define SCCB_HAECC2 0xA0
#define SCCB_HAECC3 0xA6
#define SCCB_HAECC4 0xA7
#define SCCB_HAECC5 0xA8
#define SCCB_HAECC6 0xA9
#define SCCB_HAECC7 0xAA
#define SCCB_MVFP 0x1E
#define SCCB_DVLB 0x6B
#define SCCB_REG74 0x74
#define SCCB_ABLC 0xB1 
#define SCCB_THL_ST 0xB3

#define SCCB_AWBC1 0x43
#define SCCB_AWBC2 0x44
#define SCCB_AWBC3 0x45
#define SCCB_AWBC4 0x46
#define SCCB_AWBC5 0x47
#define SCCB_AWBC6 0x48
#define SCCB_AWBCTR3 0x6C
#define SCCB_AWBCTR2 0x6D
#define SCCB_AWBCTR1 0x6E
#define SCCB_AWBCTR0 0x6F
#define SCCB_GGAIN 0x6A
#define SCCB_BLUE 0x01
#define SCCB_RED 0x02

#define SCCB_REG75 0x75
#define SCCB_REG76 0x76
#define SCCB_REG77 0x77
#define SCCB_REG4B 0x4B
#define SCCB_SATCTR 0xC9
#define SCCB_CONTRAS 0x56
#define SCCB_ARBLM 0x34
#define SCCB_NT_CTRL 0xA4
#define SCCB_BD50ST 0x9D
#define SCCB_BD60ST 0x9E

#define SCCB_SLOP 0x7A
#define SCCB_GAM1 0x7B
#define SCCB_GAM2 0x7C
#define SCCB_GAM3 0x7D
#define SCCB_GAM4 0x7E
#define SCCB_GAM5 0x7F
#define SCCB_GAM6 0x80
#define SCCB_GAM7 0x81
#define SCCB_GAM8 0x82
#define SCCB_GAM9 0x83
#define SCCB_GAM10 0x84
#define SCCB_GAM11 0x85
#define SCCB_GAM12 0x86
#define SCCB_GAM13 0x87
#define SCCB_GAM14 0x88
#define SCCB_GAM15 0x89

#define SCCB_MTX1 0x4F
#define SCCB_MTX2 0x50
#define SCCB_MTX3 0x51
#define SCCB_MTX4 0x52
#define SCCB_MTX5 0x53
#define SCCB_MTX6 0x54
#define SCCB_MTXS 0x58

#define SCCB_EDGE 0x3F

/* bits */

// COM3
#define bmSCALE_ENABLE bmBIT3
#define bmMSB_SWAP bmBIT6

// COM7
#define bmCIF  bmBIT5
#define bmQVGA bmBIT4
#define bmQCIF bmBIT3
#define bmSCCB_RESET bmBIT7

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

// ABLC1
#define bmABLC bmBIT2

// COM13
#define bmGAMMA bmBIT7
#define bmUVSAT bmBIT6

// COM16
#define bmDENOISE bmBIT4

// COM1
#define bmCIR656 bmBIT6

// COM8
#define bmFAST_AGCAEC bmBIT7
#define bmUNLIMITIED_AEC bmBIT6
#define bmBANDING_ON bmBIT5
#define bmAGC bmBIT2
#define bmAWB bmBIT1
#define bmAEC bmBIT0

// DVLB
#define bmPLL1 bmBIT7
#define bmPLL0 bmBIT6
#define bmBYPASS_REG bmBIT4

// REG74
#define bmDGAIN_REG74 bmBIT4

// COM11
#define bmEXPTIM bmBIT1
#define bmHZAUTO bmBIT4
#endif
