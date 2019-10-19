#include "defines.h"

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "descriptors.h"

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

/*
	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) // class request type 
	{	

		//  Prepare bulk-in endpoint to respond to early termination  
		
		if ((rq->bmRequestType & USBRQ_DIR_MASK) == USBRQ_DIR_HOST_TO_DEVICE)
			sendEmptyFrame = 1;
	}
*/
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
	uchar midiMsg[4];
	uchar but;
	
	PORTB |= (1 << BUT_B0) | (1 << BUT_B1) | (1 << BUT_B2) | (1 << BUT_B3);
	DDRB &= ~(1 << BUT_B0) & ~(1 << BUT_B1) & ~(1 << BUT_B2) & ~(1 << BUT_B3);
	
	PORTC |= (1 << BUT_C0) | (1 << BUT_C1);
	DDRC &= ~(1 << BUT_C0) & ~(1 << BUT_C1);
	
	PORTD &= ~(1 << LED_D0) & ~(1 << LED_D1) & ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT);
	PORTD |= (1 << BUT_D0) | (1 << BUT_D1);
	
	DDRD |= (1 << LED_D0) | (1 << LED_D1);
	DDRD &= ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT) & ~(1 << BUT_D0) & ~(1 << BUT_D1);	
	
	//usbDeviceDisconnect();
	//for(i = 0; i <= )
	
	usbDeviceConnect();
	usbInit();
    
	sei();
	while (1) 
    {
		usbPoll();
		
		if(usbInterruptIsReady())
		{
			but = PINB;
			if((but & 0x01) == 0x00)
			{
				midiMsg[0] = 0x09;
				midiMsg[1] = 0x90; // channel for "Note on" event: 0b1001_NNNN, NNNN - number of channel
				midiMsg[2] = 0x3C; // code note
				midiMsg[3] = 0x7F; // velocity
				PORTD ^= (1 << LED_D1);
			}
			else
			{
				midiMsg[0] = 0x08;
				midiMsg[1] = 0x80; // "Note off" event: 0b1000_NNNN
				midiMsg[2] = 0x3C;
				midiMsg[3] = 0x00;
				
			}
			usbSetInterrupt(midiMsg, 4);
		}
		
ERROR:	if(curInt == 0x00) 
		{
			PORTD |= (1 << LED_D0) | (1 << LED_D1); // wrong interface: required reset
			usbPoll();
			goto ERROR;
		}
		
		_delay_ms(3);
    }
}

