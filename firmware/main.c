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
  /* configure fifos */

  // enable FIFO configuration
  REVCTL = 3; SYNCDELAY;

  // reset all fifos
  FIFORESET = 0x80; SYNCDELAY;						// From now on, NAK all, reset all FIFOS
  FIFORESET  = 0x02; SYNCDELAY;					// Reset FIFO 2
  FIFORESET  = 0x04; SYNCDELAY;					// Reset FIFO 4
  FIFORESET  = 0x06; SYNCDELAY;					// Reset FIFO 6
  FIFORESET  = 0x08; SYNCDELAY;					// Reset FIFO 8
  FIFORESET  = 0x00; SYNCDELAY;					// Restore normal behaviour
  
  // disable unused endpoints
  EP1OUTCFG &= ~bmVALID; SYNCDELAY;
  EP1INCFG  &= ~bmVALID; SYNCDELAY;
  EP2CFG &= ~bmVALID; SYNCDELAY;
  EP4CFG &= ~bmVALID; SYNCDELAY;
  EP8CFG &= ~bmVALID; SYNCDELAY;
  
  EP6CFG = bmVALID | bmISOCHRONOUS | bmIN | bmQUADBUF;
  
  /*
  EP2FIFOCFG = 0x00; SYNCDELAY;					// Endpoint 2
  EP2CFG     = 0xA2; SYNCDELAY;					// Endpoint 2 Valid, Out, Type Bulk, Double buffered

  EP4FIFOCFG = 0x00; SYNCDELAY;					// Endpoint 4 not used
  EP4CFG     = 0xA0; SYNCDELAY;					// Endpoint 4 not used

  // disable FIFO access/configuration, enable auto-arming when AUTOOUT is switched to 1
  REVCTL = 0; SYNCDELAY;

  EP6CFG     = 0xA2; SYNCDELAY;					// Out endpoint, Bulk, Double buffering
  EP6FIFOCFG = 0x00; SYNCDELAY;					// Firmware has to see a rising edge on auto bit to enable auto arming
  EP6FIFOCFG = bmAUTOOUT | bmWORDWIDE; SYNCDELAY;	// Endpoint 6 used for user communicationn, auto commitment, 16 bits data bus

  EP8CFG     = 0xE0; SYNCDELAY;					// In endpoint, Bulk
  EP8FIFOCFG = 0x00; SYNCDELAY;					// Firmware has to see a rising edge on auto bit to enable auto arming
  EP8FIFOCFG = bmAUTOIN  | bmWORDWIDE; SYNCDELAY;	// Endpoint 8 used for user communication, auto commitment, 16 bits data bus

  EP8AUTOINLENH = 0x00; SYNCDELAY;					// Size in bytes of the IN data automatically commited (64 bytes here, but changed dynamically depending on the connection)
  EP8AUTOINLENL = 0x40; SYNCDELAY;					// Can use signal PKTEND if you want to commit a shorter packet

  // Out endpoints do not come up armed
  // Since the defaults are double buffered we must write dummy byte counts twice
  EP2BCL = 0x80; SYNCDELAY;						// Arm EP2OUT by writing byte count w/skip.=
  EP4BCL = 0x80; SYNCDELAY;
  EP2BCL = 0x80; SYNCDELAY;						// Arm EP4OUT by writing byte count w/skip.= 
  EP4BCL = 0x80; SYNCDELAY;
  */
  // Put the FIFO in sync mode
  //IFCONFIG &= ~bmASYNC;
}

unsigned char app_vendor_cmd(void)
{
  #if 0
  // OUT requests. Pretend we handle them all...
  if ((bRequestType & bmRT_DIR_MASK) == bmRT_DIR_OUT){
      if(bRequest == RQ_GET_STATUS){
          Running = 1;
      };
      return 1;
  }

  // IN requests.    
  switch (bRequest){
    /*case 0x94: // get Firmware version
    {
      int i=0;
      char* ver=FWVERSION;
      while(ver[i]!='\0'){
        EP0BUF[i]=ver[i];
        i++;
      }
      EP0BCH = 0; // Arm endpoint
      EP0BCL = i; 
      
    }
    break;*/
    default:
      return 1; // TODO: this should stall, but it seems I have to unstall later
    /*{
      // dummy data
      EP0BUF[0] = 0x36;
      EP0BUF[1] = 0x83;
      EP0BCH = 0;
      EP0BCL = (wLengthL<2) ? wLengthL : 2;
    }*/
    break;
  }

  #endif
  return 1;
}




