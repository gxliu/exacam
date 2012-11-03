/*-----------------------------------------------------------------------------
 * Code that turns a Cypress FX2 USB Controller into an USB JTAG adapter
 *-----------------------------------------------------------------------------
 * Copyright (C) 2005..2007 Kolja Waschk, ixo.de
 *-----------------------------------------------------------------------------
 * Check hardware.h/.c if it matches your hardware configuration (e.g. pinout).
 * Changes regarding USB identification should be made in product.inc!
 *-----------------------------------------------------------------------------
 * This code is part of usbjtag. usbjtag is free software; you can redistribute
 * it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version. usbjtag is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.  You should have received a
 * copy of the GNU General Public License along with this program in the file
 * COPYING; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *-----------------------------------------------------------------------------
 */
 
#include <isr.h>
#include <timer.h>
#include <delay.h>
#include <fx2regs.h>
#include <fx2utils.h>
#include <usb_common.h>
#include <usb_descriptors.h>
#include <usb_requests.h>
#include <syncdelay.h>
#include <i2c.h>
#include <sccb.h>
#include "config.h"

char capturing = 0;
char start = 0;
char stop = 0;

static void isr_ex0 (void) interrupt
{
  FIFORESET = 0x02; SYNCDELAY;					// Reset FIFO 2
  FIFORESET = 0x0; SYNCDELAY;          // Release NAK-all
  IOA3 ^= 1; SYNCDELAY;                // Stop ignoring camera
  EX0 = 1; SYNCDELAY;                  // Stop further interrupts
  IE0 = 0; SYNCDELAY;                  // clear flag
}

/* declarations */
void firmware_initialize(void);

void main(void)
{
  /* initialize */
  EA = 0; SYNCDELAY; // disable all interrupts
  firmware_initialize();
  setup_autovectors ();
  usb_install_handlers ();
  EA = 1; SYNCDELAY; // enable interrupts
  
  /* renumerate */
  fx2_renumerate();

  /* main loop */
  while(1)
  {
    /* check for vendor commands */
    if (usb_setup_packet_avail())
      usb_handle_setup_packet();
  }
}

#define VSTART 12
#define VSTOP 492
#define HSTART 168
#define HSTOP 24


void firmware_initialize(void)
{
  char clkrc, mtxs;
  
  FIFORESET = 0x80; SYNCDELAY;					// NAK all transfers until the frame starts
  
  /* interrupts */
  IT0 = 1;            // INT0 on low-edge (otherwise INT is repeated during low-level)
  EX0 = 0; SYNCDELAY; // disable INT0 interrupt
  IE0 = 0; SYNCDELAY; // clear INT0 flag      
  hook_sv(SV_INT_0,(unsigned short) isr_ex0);
  
  /* configure system */
  CPUCS |= bmCLKSPD1; SYNCDELAY; // set 24MHz clock + output clock (todo: test 48MHz)
  
  /* configure camera (needs to be done here so that the IFCLK is active before configuring FIFOs) */
  PORTACFG = bmINT0 | bmSLCS; SYNCDELAY; // only PA0 as INT0 
  OEA |= bmBIT3 | bmBIT1; SYNCDELAY; // PA1 (camera reset) and PA3 (camera PWDN) as outputs, PA7 (led)
  IOA1 = 1; mdelay(1); // camera reset high (put camera out of reset) -> TODO: tie to always out of reset, reset from SCCB
  IOA3 = 1; SYNCDELAY;// enable SLCS, ignore camera
  
  /* configure camera registers */
  sccb_modify(SCCB_COM7, bmSCCB_RESET, 0); mdelay(1); // reset

  sccb_modify(SCCB_CLKRC, 3, bmNO_PRESCALE); // set clock prescale. 0 = CLKIN / 2, 1 = CLKIN / 4 (w/rsvd)
  clkrc = sccb_read(SCCB_CLKRC);
  //sccb_modify(SCCB_TSLB, 0, bmBIT3); // (w/rsvd)
  
  sccb_modify(SCCB_COM7, bmQVGA | /*bmCOLORBAR |*/ bmRGB, 0);
  sccb_modify(SCCB_RGB444, 0, bmBIT1);
  //sccb_modify(SCCB_COM1, bmCIR656, 0); 
  sccb_modify(SCCB_COM15, bmBIT4, 0); // RGB565  
  sccb_write(SCCB_COM9, 0x38);
  
  sccb_modify(SCCB_COM3, bmSCALE_ENABLE, 0);  
  //sccb_modify(SCCB_SCALING_XSC, bmTEST_PATTERN, 0);
  //sccb_modify(SCCB_SCALING_YSC, bmTEST_PATTERN, 0);
  
  sccb_modify(SCCB_ABLC1, bmABLC, 0);
  sccb_modify(SCCB_COM10, bmNEG_VSYNC, 0); // invert VSYNC
  
  /*// 12Mhz PCLK
  sccb_modify(SCCB_COM14, bmDCWSCALE | 0b10, 0); 
  sccb_modify(SCCB_SCALING_PCLK_DIV, bmDSPSCALE | 0b10, 0);*/
  
  sccb_modify(SCCB_COM16, bmDENOISE, 0);
  
  /* gamma curve */
  sccb_write(SCCB_SLOP, 0x20);
  sccb_write(SCCB_GAM1, 0x10);
  sccb_write(SCCB_GAM2, 0x1e);
  sccb_write(SCCB_GAM3, 0x35);
  sccb_write(SCCB_GAM4, 0x5a);
  sccb_write(SCCB_GAM5, 0x69);
  sccb_write(SCCB_GAM6, 0x76);
  sccb_write(SCCB_GAM7, 0x80);
  sccb_write(SCCB_GAM8, 0x88);
  sccb_write(SCCB_GAM9, 0x8f);
  sccb_write(SCCB_GAM10, 0x96);
  sccb_write(SCCB_GAM11, 0xa3);
  sccb_write(SCCB_GAM12, 0xaf);
  sccb_write(SCCB_GAM13, 0xc4);
  sccb_write(SCCB_GAM14, 0xd7);
  sccb_write(SCCB_GAM15, 0xe8);
  
  /* exposure, etc. */
  sccb_write(SCCB_COM8, bmFAST_AGCAEC | bmUNLIMITIED_AEC | bmBANDING_ON);
  sccb_write(SCCB_GAIN, 0);
  sccb_write(SCCB_AECH, 0);
  sccb_write(SCCB_COM4, 0x40); /* magic reserved bit */
  sccb_write(SCCB_COM9, 0x18); /* 4x gain + magic rsvd bit */
  sccb_write(SCCB_BD50MAX, 0x05);
  sccb_write(SCCB_BD60MAX, 0x07);
  sccb_write(SCCB_AEW, 0x95);
  sccb_write(SCCB_AEB, 0x33);
  sccb_write(SCCB_VPT, 0xe3);
  sccb_write(SCCB_HAECC1, 0x78);
  sccb_write(SCCB_HAECC2, 0x68);
  sccb_write(0xa1, 0x03); /* magic */
  sccb_write(SCCB_HAECC3, 0xd8);
  sccb_write(SCCB_HAECC4, 0xd8);
  sccb_write(SCCB_HAECC5, 0xf0);
  sccb_write(SCCB_HAECC6, 0x90);
  sccb_write(SCCB_HAECC7, 0x94);
  sccb_write(SCCB_COM8, bmFAST_AGCAEC | bmUNLIMITIED_AEC | bmBANDING_ON | bmAGC | bmAEC);
  
  /* Almost all of these are magic "reserved" values.  */
  sccb_write(SCCB_COM5, 0x61);
  sccb_write(SCCB_COM6, 0x4b);
  sccb_write(0x16, 0x02);
  sccb_write(SCCB_MVFP, 0x07); // NOTE: black-sun enabled
  sccb_write(0x21, 0x02);
  sccb_write(0x22, 0x91);
  sccb_write(0x29, 0x07);
  sccb_write(0x33, 0x0b);
  sccb_write(0x35, 0x0b);
  sccb_write(0x37, 0x1d);
  sccb_write(0x38, 0x71);
  sccb_write(0x39, 0x2a);
  sccb_write(SCCB_COM12, 0x78);
  sccb_write(0x4d, 0x40);
  sccb_write(0x4e, 0x20);
  sccb_write(SCCB_GFIX, 0);
  // sccb_write(SCCB_DBLV, 0x4a); DONT ENABLE -> PLL x4
  sccb_write(SCCB_REG74, bmDGAIN_REG74);
  sccb_write(0x8d, 0x4f);
  sccb_write(0x8e, 0);
  sccb_write(0x8f, 0);
  sccb_write(0x90, 0);
  sccb_write(0x91, 0);
  sccb_write(0x96, 0);
  sccb_write(0x9a, 0);
  sccb_write(0xb0, 0x84);
  sccb_write(SCCB_ABLC, 0x0c);
  sccb_write(0xb2, 0x0e);
  sccb_write(SCCB_THL_ST, 0x82);
  sccb_write(0xb8, 0x0a);
  
  /* WB magic values */
  sccb_write(SCCB_AWBC1, 0x0a);
  sccb_write(SCCB_AWBC2, 0xf0);
  sccb_write(SCCB_AWBC3, 0x34);
  sccb_write(SCCB_AWBC4, 0x58);
  sccb_write(SCCB_AWBC5, 0x28);
  sccb_write(SCCB_AWBC6, 0x3a);
  sccb_write(0x59, 0x88);
  sccb_write(0x5a, 0x88);
  sccb_write(0x5b, 0x44);
  sccb_write(0x5c, 0x67);
  sccb_write(0x5d, 0x49);
  sccb_write(0x5e, 0x0e);
  sccb_write(SCCB_AWBCTR3, 0x0a);
  sccb_write(SCCB_AWBCTR2, 0x55);
  sccb_write(SCCB_AWBCTR1, 0x11);
  sccb_write(SCCB_AWBCTR0, 0x9f); /* "9e for advance AWB" */
  sccb_write(SCCB_GGAIN, 0x40);
  sccb_write(SCCB_BLUE, 0x40);
  sccb_write(SCCB_RED, 0x60);
  sccb_write(SCCB_COM8, bmFAST_AGCAEC | bmUNLIMITIED_AEC | bmBANDING_ON | bmAGC | bmAEC | bmAWB);
  
  /* color matrix */
  sccb_write(SCCB_MTX1, 179);
  sccb_write(SCCB_MTX2, 179);
  sccb_write(SCCB_MTX3, 0);
  sccb_write(SCCB_MTX4, 61);
  sccb_write(SCCB_MTX5, 176);
  sccb_write(SCCB_MTX6, 228);
  mtxs = sccb_read(SCCB_MTXS);
  sccb_write(SCCB_MTXS, (mtxs & 0xC0) | 26);
  sccb_modify(SCCB_COM13, bmUVSAT | bmGAMMA, 0);
  
  //sccb_write(REG_COM16, COM16_AWBGAIN);
  sccb_write(SCCB_EDGE, 0);
  sccb_write(SCCB_REG75, 0x05);
  sccb_write(SCCB_REG76, 0xe1);
  //sccb_write(0x4c, 0);
  sccb_write(SCCB_REG77, 0x01);
  sccb_write(SCCB_COM13, 0xc3);
  sccb_write(SCCB_REG4B, 0x09);
  sccb_write(SCCB_SATCTR, 0x60);
  sccb_write(SCCB_COM16, 0x38);
  sccb_write(SCCB_CONTRAS, 0x40);
  sccb_write(SCCB_ARBLM, 0x11);
  sccb_write(SCCB_COM11, bmEXPTIM | bmHZAUTO);
  sccb_write(SCCB_NT_CTRL, 0x88);
  sccb_write(0x96, 0);
  sccb_write(0x97, 0x30);
  sccb_write(0x98, 0x20);
  sccb_write(0x99, 0x30);
  sccb_write(0x9a, 0x84);
  sccb_write(0x9b, 0x29);
  sccb_write(0x9c, 0x03);
  sccb_write(SCCB_BD50ST, 0x4c);
  sccb_write(SCCB_BD60ST, 0x3f);
  sccb_write(0x78, 0x04);
  
  /* do not know */
  sccb_write(0x79, 0x01);
  sccb_write(0xc8, 0xf0);
  sccb_write(0x79, 0x0f);
  sccb_write(0xc8, 0x00);
  sccb_write(0x79, 0x10);
  sccb_write(0xc8, 0x7e);
  sccb_write(0x79, 0x0a);
  sccb_write(0xc8, 0x80);
  sccb_write(0x79, 0x0b);
  sccb_write(0xc8, 0x01);
  sccb_write(0x79, 0x0c);
  sccb_write(0xc8, 0x0f);
  sccb_write(0x79, 0x0d);
  sccb_write(0xc8, 0x20);
  sccb_write(0x79, 0x09);
  sccb_write(0xc8, 0x80);
  sccb_write(0x79, 0x02);
  sccb_write(0xc8, 0xc0);
  sccb_write(0x79, 0x03);
  sccb_write(0xc8, 0x40);
  sccb_write(0x79, 0x05);
  sccb_write(0xc8, 0x30);
  sccb_write(0x79, 0x26);
  
  sccb_write(SCCB_CLKRC, clkrc);
  
  /* 320x240 */
  sccb_write(SCCB_VREF, (VSTART & 0b11) | ((VSTOP & 0b11) << 2));
  sccb_write(SCCB_VSTART, (VSTART & 0b1111111100) >> 2);
  sccb_write(SCCB_VSTOP, (VSTOP & 0b1111111100) >> 2);
  sccb_write(SCCB_HREF, (HSTART & 0b111) | ((HSTOP & 0b111) << 3));
  sccb_write(SCCB_HSTART, (HSTART & 0b11111111000) >> 3);
  sccb_write(SCCB_HSTOP, (HSTOP & 0b11111111000) >> 3);
  mdelay(300); // wait 300ms for register settle
  
  /* configure fifos */
  IFCONFIG &= ~bmIFCLKSRC; SYNCDELAY; // disable int. FIFO clock (use ext. clock)
  IFCONFIG |= (bmIFCFG0 | bmIFCFG1); SYNCDELAY; // enable SLAVE FIFO mode

  REVCTL = 3; SYNCDELAY; // enable FIFO configuration
  
  // disable unused endpoints
  EP1OUTCFG &= ~bmVALID; SYNCDELAY;
  EP1INCFG  &= ~bmVALID; SYNCDELAY;
  EP6CFG &= ~bmVALID; SYNCDELAY;
  EP4CFG &= ~bmVALID; SYNCDELAY;
  EP8CFG &= ~bmVALID; SYNCDELAY;
  
  // enable EP2
  EP2CFG = bmVALID | bmISOCHRONOUS | bmIN | bmQUADBUF | bm1KBUF; SYNCDELAY;
  
  // setup EP2
  EP2FIFOCFG &= ~bmWORDWIDE; SYNCDELAY; // set EP2 8bits (PORTB -> FD[7:0]) 
  EP2FIFOCFG |= bmAUTOIN; SYNCDELAY;
  
  //EP2ISOINPKTS |= bmBIT1 | bmBIT0; // set INPPF[1:0] = 3 -> 3 packets per microframe
  // TODO: tuned for QVGA -> frame size seems to be 313x238
  EP2AUTOINLENH = (640 >> 8); SYNCDELAY;
  EP2AUTOINLENL = (640 & 0xFF); SYNCDELAY;
  FIFOINPOLAR |= bmBIT2; SYNCDELAY; // set SLWR active-high
    
  // disable access to FIFOs
  REVCTL = 0; SYNCDELAY;  
}

unsigned char app_vendor_cmd(void)
{
  // OUT requests. Pretend we handle them all...
  // TODO: what is this?
  if ((bRequestType & bmRT_DIR_MASK) == bmRT_DIR_OUT){
    return 1;
  }

  // IN requests.    
  switch (bRequest){
    // get firmware version / git revision
    case 0x94: 
    case 0x95:
    {
      int i = 0;
      const char* ver = (bRequest == 0x94 ? FIRMWARE_VERSION : FIRMWARE_GIT_REVISION);
      while (ver[i] != '\0') {
        EP0BUF[i] = ver[i];
        i++;
      }
      EP0BCH = 0; // Arm endpoint
      EP0BCL = i;
    }
    break;
    case 0x96: // start capture
    {
      IE0 = 0; SYNCDELAY;
      EX0 = 1; SYNCDELAY; // enable INT0 interrupt
    }
    break;
    default:
      // dummy data
      EP0BUF[0] = 0x36;
      EP0BUF[1] = 0x83;
      EP0BCH = 0;
      EP0BCL = (wLengthL<2) ? wLengthL : 2;
    break;
  }

  return 1;
}




