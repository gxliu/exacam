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
 
#define FWVERSION "4.2.0"

#include <isr.h>
#include <timer.h>
#include <delay.h>
#include <fx2regs.h>
#include <fx2utils.h>
#include <usb_common.h>
#include <usb_descriptors.h>
#include <usb_requests.h>
#include <syncdelay.h>

//-----------------------------------------------------------------------------
// Define USE_MOD256_OUTBUFFER:
// Saves about 256 bytes in code size, improves speed a little.
// A further optimization could be not to use an extra output buffer at 
// all, but to write directly into EP1INBUF. Not implemented yet. When 
// downloading large amounts of data _to_ the target, there is no output
// and thus the output buffer isn't used at all and doesn't slow down things.

#define USE_MOD256_OUTBUFFER 1

//-----------------------------------------------------------------------------
// Global data

typedef bit BOOL;
#define FALSE 0
#define TRUE  1
static BOOL Running;
static BOOL WriteOnly;

static BYTE ClockBytes;
static WORD Pending;

#ifdef USE_MOD256_OUTBUFFER
  static BYTE FirstDataInOutBuffer;
  static BYTE FirstFreeInOutBuffer;
#else
  static WORD FirstDataInOutBuffer;
  static WORD FirstFreeInOutBuffer;
#endif

#ifdef USE_MOD256_OUTBUFFER
  /* Size of output buffer must be exactly 256 */
  #define OUTBUFFER_LEN 0x100
  /* Output buffer must begin at some address with lower 8 bits all zero */
  xdata at 0xE000 BYTE OutBuffer[OUTBUFFER_LEN];
#else
  #define OUTBUFFER_LEN 0x200
  static xdata BYTE OutBuffer[OUTBUFFER_LEN];
#endif

//-----------------------------------------------------------------------------

void usb_jtag_init(void)              // Called once at startup
{
   WORD tmp;

   Running = FALSE;
   ClockBytes = 0;
   Pending = 0;
   WriteOnly = TRUE;
   FirstDataInOutBuffer = 0;
   FirstFreeInOutBuffer = 0;

   //ProgIO_Init();

   //ProgIO_Enable();

   // Make Timer2 reload at 100 Hz to trigger Keepalive packets
   tmp = 65536 - ( 48000000 / 12 / 100 );
   RCAP2H = tmp >> 8;
   RCAP2L = tmp & 0xFF;
   CKCON = 0; // Default Clock
   T2CON = 0x04; // Auto-reload mode using internal clock, no baud clock.

   // Enable Autopointer
   EXTACC = 1;		// Enable
   APTR1FZ = 1;		// Don't freeze
   APTR2FZ = 1;		// Don't freeze

   // define endpoint configuration

   REVCTL = 3; SYNCDELAY;							// Allow FW access to FIFO buffer
   FIFORESET = 0x80; SYNCDELAY;						// From now on, NAK all, reset all FIFOS
   FIFORESET  = 0x02; SYNCDELAY;					// Reset FIFO 2
   FIFORESET  = 0x04; SYNCDELAY;					// Reset FIFO 4
   FIFORESET  = 0x06; SYNCDELAY;					// Reset FIFO 6
   FIFORESET  = 0x08; SYNCDELAY;					// Reset FIFO 8
   FIFORESET  = 0x00; SYNCDELAY;					// Restore normal behaviour

   EP1OUTCFG  = 0xA0; SYNCDELAY;					// Endpoint 1 Type Bulk			
   EP1INCFG   = 0xA0; SYNCDELAY;					// Endpoint 1 Type Bulk

   EP2FIFOCFG = 0x00; SYNCDELAY;					// Endpoint 2
   EP2CFG     = 0xA2; SYNCDELAY;					// Endpoint 2 Valid, Out, Type Bulk, Double buffered

   EP4FIFOCFG = 0x00; SYNCDELAY;					// Endpoint 4 not used
   EP4CFG     = 0xA0; SYNCDELAY;					// Endpoint 4 not used

   REVCTL = 0; SYNCDELAY;							// Reset FW access to FIFO buffer, enable auto-arming when AUTOOUT is switched to 1

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
   
   // JTAG from FX2 enabled by default
   IOC |= (1 << 7);
   
   // Put the system in high speed by default (REM: USB-Blaster is in full speed)
   // This can be changed by vendor commands
   CT1 &= ~0x02;
   // Put the FIFO in sync mode
   IFCONFIG &= ~bmASYNC;
}

//-----------------------------------------------------------------------------
// Handler for Vendor Requests (
//-----------------------------------------------------------------------------

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
    case 0x90: // Read EEPROM
        { // We need a block for addr
            BYTE addr = (wIndexL<<1) & 0x7F;
            EP0BUF[0] = eeprom[addr];
            EP0BUF[1] = eeprom[addr+1];
            EP0BCL = (wLengthL<2) ? wLengthL : 2; 
        }
        break;
        
    case 0x91: // change USB speed
        if (wIndexL == 0){			// high speed
            CT1 &= ~0x02;
            EP0BUF[0] = 0x2;
            fx2_renumerate();		// renumerate
        }else{						// full speed
            CT1 |= 0x02;
            EP0BUF[0] = 0x1;
            fx2_renumerate();		// renumerate
        }
        EP0BCH = 0; // Arm endpoint
        EP0BCL = 1; // # bytes to transfer
        break;
        
    case 0x92: // change JTAG enable
        if (wIndexL == 0){			// FX2 is master of JTAG
            IOC |= (1 << 7);
        }else{						// external connector is master of JTAG
            IOC &= ~(1 << 7);
        }
        EP0BCH = 0; // Arm endpoint
        EP0BCL = 0; // # bytes to transfer
        break;
        
    case 0x93: // change synchronous/asynchronous mode
        if(wIndexL == 0){           // sync
            IFCONFIG &= ~bmASYNC;
            EP0BUF[0] = 0;
        }else{
            IFCONFIG |= bmASYNC;    // async
            EP0BUF[0] = 1;
        }
        EP0BCH = 0; // Arm endpoint
        EP0BCL = 1; 
        break;
    
    case 0x94: // get Firmware version
        {
          int i=0;
          char* ver=FWVERSION;
          while(ver[i]!='\0'){
            EP0BUF[i]=ver[i];
            i++;
          }
          EP0BCH = 0; // Arm endpoint
          EP0BCL = i; 
          break;
        }
    default:
        // dummy data
        EP0BUF[0] = 0x36;
        EP0BUF[1] = 0x83;
        EP0BCH = 0;
        EP0BCL = (wLengthL<2) ? wLengthL : 2;
    }
  
  #endif
  return 1;
}

//-----------------------------------------------------------------------------

static void main_loop(void)
{
  while(1)
  {
    if(usb_setup_packet_avail()) usb_handle_setup_packet();
    //usb_jtag_activity();
  }
}

//-----------------------------------------------------------------------------

void main(void)
{
  EA = 0; // disable all interrupts

  /*usb_jtag_init();
  eeprom_init();*/
  setup_autovectors ();
  usb_install_handlers ();


  EA = 1; // enable interrupts

  fx2_renumerate(); // simulates disconnect / reconnect

  main_loop();
}




