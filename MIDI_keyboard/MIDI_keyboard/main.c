#include "defines.h"

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "usbdrv/usbdrv.h"

const static uchar PROGMEM devDesc[] = {
	0x12,					/* sizeof(usbDescriptorDevice): length of descriptor in bytes */
	USBDESCR_DEVICE,		/* descriptor type */
	0x10, 0x01,				/* USB version supported */
	0x00,					/* device class: defined at interface level */
	0x00,					/* subclass */
	0x00,					/* protocol */
	0x08,					/* max packet size */
	USB_CFG_VENDOR_ID,		/* 2 bytes */
	USB_CFG_DEVICE_ID,		/* 2 bytes */
	USB_CFG_DEVICE_VERSION,	/* 2 bytes */
	0x01,					/* manufacturer string index */
	0x02,					/* product string index */
	0x00,					/* serial number string index */
	0x01,					/* number of configurations */
};

const static uchar PROGMEM confDesc[] = {
	
				/****************** Configuration Descriptor ******************/
	
	0x09,						/* sizeof(usbDescrConfig): length of descriptor in bytes */
	USBDESCR_CONFIG,			/* descriptor type */
	0x65, 0x00,					/* total length of data returned (including inlined descriptors) */
	0x02,						/* number of interfaces in this configuration */
	0x01,						/* index of this configuration */
	0x00,						/* configuration name string index */
	USBATTR_BUSPOWER,			/* attributes */
	USB_CFG_MAX_BUS_POWER / 2,	/* max USB current in 2mA units */
	
				/************** AudioControl Interface Descriptor **************/
	
	// Standard AC Interface Descriptor:
	0x09,					/* sizeof(usbDescrInterface): length of descriptor in bytes */
	USBDESCR_INTERFACE,		/* descriptor type */
	0x00,					/* index of this interface */
	0x00,					/* alternate setting for this interface */
	0x00,					/* endpoints excl 0: number of endpoint descriptors to follow */
	0x01,					/* interface class: AUDIO */
	0x01,					/* interface subclass: AUDIO_CONTROL */
	0x00,					/* interface protocol */
	0x00,					/* string index for interface */

	// Class-specific AC Interface Descriptor (header only, cause audio functionality does not contain):
	0x09,					/* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x01,					/* descriptor subtype: header functional descriptor */
	0x00, 0x01,				/* bcdADC: revision of class specification - 1.0 */
	0x09, 0x00,				/* wTotalLength */
	0x01,					/* number of streaming interfaces */
	0x01,					/* MIDIStreaming interface 1 belongs to this AudioControl interface */

				/************* MIDIStreaming Interface Descriptors *************/

	// Standard MS Interface Descriptor:
	0x09,					/* length of descriptor in bytes */
	USBDESCR_INTERFACE,		/* descriptor type */
	0x01,					/* index of this interface */
	0x00,					/* alternate setting for this interface */
	0x02,					/* endpoints excl 0: number of endpoint descriptors to follow */
	0x01,					/* interface class: AUDIO */
	0x03,					/* interface subclass: MIDIStreaming */
	0x00,					/* interface protocol */
	0x00,					/* string index for interface */
		
	// Class-specific MS Interface Descriptor:
	0x07,					/* length of descriptor in bytes */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x01,					/* descriptor subtype: header functional descriptor */
	0x00, 0x01,				/* bcdADC: revision of class specification */
	0x41, 0x00,				/* total size of class-specific descriptors */

	// MIDI IN Jack Descriptor:
	0x06,					/* size of this descriptor */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x02,					/* descriptor subtype: MIDI_IN_JACK */
	0x01,					/* jackType: EMBEDDED */
	0x01,					/* id of this jack */
	0x00,					/* iJack */

	// MIDI Adapter MIDI IN Jack Descriptor (External):
	0x06,					/* size of this descriptor */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x02,					/* descriptor subtype: MIDI_IN_JACK */
	0x02,					/* jackType: EXTERNAL */
	0x02,					/* id of this jack */
	0x00,					/* iJack */

	// MIDI OUT Jack Descriptor:
	0x09,					/* length of descriptor in bytes */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x03,					/* descriptor subtype: MIDI_OUT_JACK */
	0x01,					/* jackType: EMBEDDED */
	0x03,					/* id of this jack */
	0x01,					/* number of Input Pins of this Jack */
	0x02,					/* id of the Entity to which this Pin is connected: BaSourceID */
	0x01,					/* output Pin number of the Entity to which this Input Pin is connected: BaSourcePin */
	0x00,					/* iJack */

	// MIDI Adapter MIDI OUT Jack Descriptor (External):
	0x09,					/* length of descriptor in bytes */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x03,					/* descriptor subtype: MIDI_OUT_JACK */
	0x02,					/* jackType: EXTERNAL */
	0x04,					/* id of this jack */
	0x01,					/* number of Input Pins of this Jack */
	0x01,					/* id of the Entity to which this Pin is connected: BaSourceID */
	0x01,					/* output Pin number of the Entity to which this Input Pin is connected: BaSourcePin */
	0x00,					/* iJack */

				/************* Bulk OUT Endpoint Descriptors *************/

	// Standard Bulk OUT Endpoint Descriptor:
	0x09,					/* size of this descriptor */
	USBDESCR_ENDPOINT,		/* descriptor type */
	0x01,					/* endpoint address: OUT endpoint number 1 */
	0x03,					/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	0x08, 0x00,				/* max packet size */
	10,						/* bIntervall in ms */
	0x00,					/* bRefresh */
	0x00,					/* bSyncAddress */

	// Class-specific MS Bulk OUT Endpoint Descriptor:
	0x05,					/* length of descriptor in bytes */
	USBDESCR_CS_ENDPOINT,	/* descriptor type */
	0x01,					/* descriptor subtype: MS_GENERAL */
	0x01,					/* number of embedded MIDI IN Jacks */
	0x01,					/* id of the Embedded MIDI IN Jack */

				/************* Bulk IN Endpoint Descriptors *************/

	// Standard Bulk IN Endpoint Descriptor:
	0x09,					/* size of this descriptor */
	USBDESCR_ENDPOINT,		/* descriptor type */
	0x81,					/* endpoint address: IN endpoint number 1 */
	0x03,					/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	0x08, 0x00,				/* max packet size */
	10,						/* bIntervall in ms */
	0x00,					/* bRefresh */
	0x00,					/* bSyncAddress */

	// Class-specific MS Bulk IN Endpoint Descriptor:
	0x05,					/* length of descriptor in bytes */
	USBDESCR_CS_ENDPOINT,	/* descriptor type */
	0x01,					/* descriptor subtype: MS_GENERAL */
	0x01,					/* number of embedded MIDI OUT Jacks */
	0x03,					/* id of the Embedded MIDI OUT Jack */
};

uchar curInt = 0x00;

uchar statusDev[2] = {0x00, 0x00};
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
	else return 0xFF;
}

uchar bReq;
uchar wInd;

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
				return 2;
			}
			else if((rq -> bmRequestType & USBRQ_RCPT_MASK) == USBRQ_RCPT_INTERFACE)
			{
				if((rq -> wIndex.bytes[0] == 0x00) | (rq -> wIndex.bytes[0] == 0x01))
				{
					usbMsgPtr = (int)&statusInt[0];
					return 2;
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
	PORTD ^= (1 << LED_D0) ^ (1 << LED_D1);
}

void main(void)
{
	volatile unsigned int i;
	
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
		
		for (i = 0; i <= 32766; i++) // ~ 9 ms
		{
			asm('nop');
			asm('nop');
			asm('nop');
		}
		
		PORTD ^= (1 << LED_D0) ^ (1 << LED_D1);
    }
}

