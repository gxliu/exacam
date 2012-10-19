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

#include "config.h"

/* declarations */
void firmware_initialize(void);

void main(void)
{
  /* initialize */
  EA = 0; // disable all interrupts
  firmware_initialize();
  setup_autovectors ();
  usb_install_handlers ();
  EA = 1; // enable interrupts

  /* renumerate */
  fx2_renumerate();

  /* main loop */
  while(1)
  {
    /* check for vendor commands */
    if (usb_setup_packet_avail())
      usb_handle_setup_packet();
      
    // insert main function here
  }
}


void firmware_initialize(void)
{
  /* configure system */
  CPUCS |= bmCLKSPD0; // set 24MHz clock + output clock. TODO: set at 48Mhz when I resolve pre-scaling on the camera
  
  /* configure fifos */
  IFCONFIG &= ~bmIFCLKSRC; SYNCDELAY; // disable int. FIFO clock (use ext. clock)
  IFCONFIG |= (bmIFCFG0 | bmIFCFG1); SYNCDELAY;// enable SLAVE FIFO operation
  
  // enable FIFO configuration
  REVCTL = 3; SYNCDELAY;
  
  // disable unused endpoints
  EP1OUTCFG &= ~bmVALID; SYNCDELAY;
  EP1INCFG  &= ~bmVALID; SYNCDELAY;
  EP2CFG &= ~bmVALID; SYNCDELAY;
  EP4CFG &= ~bmVALID; SYNCDELAY;
  EP8CFG &= ~bmVALID; SYNCDELAY;
  
  // enable EP6
  EP6CFG = bmVALID | bmISOCHRONOUS | bmIN | bmQUADBUF; SYNCDELAY;

  // reset all fifos
  FIFORESET = 0x80; SYNCDELAY;						// From now on, NAK all, reset all FIFOS
  FIFORESET  = 0x02; SYNCDELAY;					// Reset FIFO 2
  FIFORESET  = 0x04; SYNCDELAY;					// Reset FIFO 4
  FIFORESET  = 0x06; SYNCDELAY;					// Reset FIFO 6
  FIFORESET  = 0x08; SYNCDELAY;					// Reset FIFO 8
  FIFORESET  = 0x00; SYNCDELAY;					// Restore normal behaviour
  
  
  // setup EP6
  EP6FIFOCFG &= ~bmWORDWIDE; SYNCDELAY; // set EP6 8bits (PORTB -> FD[7:0]) 
  EP6AUTOINLENH = 0x4; SYNCDELAY; // high-order bits set to "100" => auto-len = 1024
  EP6FIFOCFG |= bmAUTOIN; SYNCDELAY; // set EP6 auto-in (auto-commits data from camera to usb)
  FIFOINPOLAR |= bmBIT2; SYNCDELAY; // set SLWR active-high
  
  // disable access to FIFOs
  REVCTL = 0; SYNCDELAY;
  
  // TODO: ver como setear las cosas para que se empiece a hacer todo recien al inicio de un frame (falling edge de VSYNC, por ej)
  // TODO: set FIFOADDR pins!
  // TODO: PKTEND to send a line directly?  
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




