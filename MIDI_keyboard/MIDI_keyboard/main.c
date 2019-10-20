#include "defines.h"

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "descriptors.h"

#define KEY_STATUS ((PINB & 0b00001111) | (PINC & 0b00110000) | (PIND & 0b11000000))
//   key num: 7 6 5 4 3 2 1 0
// PORT name: D D C C B B B B, bind by scheme
/* equivalent to this:		  
		(PINB & ((1 << BUT_B0) | (1 << BUT_B1) | (1 << BUT_B2) | (1 << BUT_B3))) | 
		(PINC & ((1 << BUT_C0) | (1 << BUT_C1))) | 
		(PIND & ((1 << BUT_D0) | (1 << BUT_D1)))	
*/

uchar curInt = 0x01;
uchar bReq;
uchar wInd;

uchar statusDev[2] = {0x00, 0x00}; // ???
const uchar statusInt[2] = {0x00, 0x00};

uchar usbFunctionDescriptor(usbRequest_t * rq)
{
	if(rq -> wValue.bytes[1] == USBDESCR_DEVICE)
	{
		usbMsgPtr = (int)devDesc;
		return sizeof(devDesc);
	}
	else if(rq -> wValue.bytes[1] == USBDESCR_CONFIG)
	{
		usbMsgPtr = (int)confDesc;
		return sizeof(confDesc);
	}
	else if((rq -> wValue.bytes[1] == USBDESCR_STRING) & (rq -> wValue.bytes[0] == 2)) // device name
	{
		usbMsgPtr = (int)prodStrDesr;
		return sizeof(prodStrDesr);
	}
	else return 0xFF;
}

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (void *) data;

	bReq = rq -> bRequest;
	wInd = rq -> wIndex.bytes[0];
	
	switch(bReq)
	{
		case USBRQ_GET_STATUS:
			if((rq -> bmRequestType & USBRQ_RCPT_MASK) == USBRQ_RCPT_DEVICE)
			{
				usbMsgPtr = (int)&statusDev[0];
				return sizeof(statusDev);
			}
			else if((rq -> bmRequestType & USBRQ_RCPT_MASK) == USBRQ_RCPT_INTERFACE)
			{
				if((rq -> wIndex.bytes[0] == 0x00) | (rq -> wIndex.bytes[0] == 0x01))
				{
					usbMsgPtr = (int)&statusInt[0];
					return sizeof(statusInt);
				}
				else return USB_NO_MSG;
			}
			else if((rq -> bmRequestType & USBRQ_RCPT_MASK) == USBRQ_RCPT_ENDPOINT)
			{
				return USB_NO_MSG;; // need to finish 
			}
			else return USB_NO_MSG;
		default: return USB_NO_MSG;
	}
}

uchar usbFunctionRead(uchar * data, uchar len)
{
	switch(bReq)
	{
		case USBRQ_GET_CONFIGURATION:
			data[0] = 0x01;
			return 1;
		case USBRQ_GET_INTERFACE:
			if(wInd == 0x00) 
			{
				data[0] = 0x00;
				return 1;
			}
			else if(wInd == 0x01) 
			{
				data[0] = 0x01;
				return 1;
			}
			else return 0xFF;
		default: return 0xFF;
	}
}

uchar usbFunctionWrite(uchar * data, uchar len)
{
	switch(bReq)
	{
		case USBRQ_SET_INTERFACE: 
			curInt = wInd;
			return 1;
		default: return 0xFF;
	}
}

void usbFunctionWriteOut(uchar * data, uchar len)
{
	// DEBUG LED
	PORTD ^= (1 << LED_D0);
}

void main(void)
{
	const uchar keyCode [8] = 
	{		// PIN-PORT -> "note"
		0x3C, //   0-B -> "C" middle octave: key code "0"
		0x3E, //   1-B -> "D"
		0x40, //   2-B -> "E"
		0x41, //   3-B -> "F"
		0x43, //   4-C -> "G"
		0x45, //   5-C -> "A"
		0x47, //   6-D -> "H"
		0x48  //   7-D -> "C"
	};
	
	uchar i;
	uchar midiMsg[4];
	
	uchar key[3]; // keys status: for "majoritar"
	uchar key_old = 0xFF; 
	uchar key_mask;
	
	uchar key_num_ch[8] = // number of keys that was changed: release (2) or press (1), don't changed (0)
							{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
	
	PORTB |= (1 << BUT_B0) | (1 << BUT_B1) | (1 << BUT_B2) | (1 << BUT_B3);
	DDRB &= ~(1 << BUT_B0) & ~(1 << BUT_B1) & ~(1 << BUT_B2) & ~(1 << BUT_B3);
	
	PORTC |= (1 << BUT_C0) | (1 << BUT_C1);
	DDRC &= ~(1 << BUT_C0) & ~(1 << BUT_C1);
	
	PORTD &= ~(1 << LED_D0) & ~(1 << LED_D1) & ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT);
	PORTD |= (1 << BUT_D0) | (1 << BUT_D1);
	
	DDRD |= (1 << LED_D0) | (1 << LED_D1);
	DDRD &= ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT) & ~(1 << BUT_D0) & ~(1 << BUT_D1);
	
	usbDeviceConnect();
	usbInit();
    
	sei();
	while (1) 
    {
		usbPoll();
		
		// keys polling:
			key[0] = KEY_STATUS;
				_delay_ms(2);
			key[1] = KEY_STATUS;
				_delay_ms(2);
			key[2] = KEY_STATUS;
		
		// send MIDI msg if keys
		if(usbInterruptIsReady() & 
			(key[0] == key[1]) & (key[1] == key[2]) & 
			(key[0] != key_old))
		{	  
			key_mask = key[0] ^ key_old;
			
			key[0] &= key_mask;
			key_old &= key_mask;
			
			for(i = 0; i <= 7; i++)
			{
				if(((key_old & 0x01) - (key[0] & 0x01)) == 1) key_num_ch[i] = 1; // key was pressed
				else if(((key_old & 0x01) - (key[0] & 0x01)) == 255) key_num_ch[i] = 2; // released
				
				key_mask >>= 1;
				key[0] >>= 1;
				key_old >>= 1;
			}
			
			for(i = 0; i <= 7; i++)
			{
				if(key_num_ch[i] == 1)
				{
					midiMsg[0] = 0x09;
					midiMsg[1] = 0x90;			// channel for "Note on" event: 0b1001_NNNN, NNNN - number of channel
					midiMsg[2] = keyCode[i];	// code note
					midiMsg[3] = 0x7F;			// velocity
					
					key_num_ch[i] = 0;
					PORTD ^= (1 << LED_D1);
				}
				else if(key_num_ch[i] == 2)
				{
					midiMsg[0] = 0x08;
					midiMsg[1] = 0x80;			// "Note off" event: 0b1000_NNNN
					midiMsg[2] = keyCode[i];
					midiMsg[3] = 0x00;
					
					key_num_ch[i] = 0;
				}
			}
			
			key_old = key[1];
			usbSetInterrupt(midiMsg, 4);
		}
		
ERROR:	if(curInt == 0x00) 
		{
			PORTD |= (1 << LED_D0) | (1 << LED_D1); // wrong interface: required reset
			usbPoll();
			goto ERROR;
		}
    }
}

