#include "defines.h"

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "usbdrv/usbdrv.h"

const static uchar PROGMEM deviceDescrMIDI[] = {
	0x12,					/* sizeof(usbDescriptorDevice): length of descriptor in bytes */
	0x01,					/* descriptor type */
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

const static uchar PROGMEM configDescrMIDI[] = {
	0x09,						/* sizeof(usbDescrConfig): length of descriptor in bytes */
	USBDESCR_CONFIG,			/* descriptor type */
	0x65, 0x00,					/* total length of data returned (including inlined descriptors) */
	0x02,						/* number of interfaces in this configuration */
	0x01,						/* index of this configuration */
	0x00,						/* configuration name string index */
	USBATTR_BUSPOWER,			/* attributes */
	USB_CFG_MAX_BUS_POWER / 2,	/* max USB current in 2mA units */
	
	
	
	0x09,				/* sizeof(usbDescrInterface): length of descriptor in bytes */
	USBDESCR_INTERFACE,	/* descriptor type */
	0,					/* index of this interface */
	0,					/* alternate setting for this interface */
	0,					/* endpoints excl 0: number of endpoint descriptors to follow */
	0x01,
	0x01,
	0,
	0,					/* string index for interface */



	0x09,		/* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
	0x24,		/* descriptor type */
	0x01,		/* header functional descriptor */
	0x00, 0x01,	/* bcdADC */
	0x09, 0x00,	/* wTotalLength */
	0x01,
	0x01,



	0x09,				/* length of descriptor in bytes */
	USBDESCR_INTERFACE,	/* descriptor type */
	0x01,				/* index of this interface */
	0,					/* alternate setting for this interface */
	0x02,				/* endpoints excl 0: number of endpoint descriptors to follow */
	0x01,				/* AUDIO */
	0x03,				/* MS */
	0,					/* unused */
	0,					/* string index for interface */



	0x07,		/* length of descriptor in bytes */
	0x24,		/* descriptor type */
	0x01,		/* header functional descriptor */
	0x00, 0x01,	/* bcdADC */
	0x41, 0x00,	/* wTotalLength */



	6,			/* bLength */
	36,			/* descriptor type */
	2,			/* MIDI_IN_JACK desc subtype */
	1,			/* EMBEDDED bJackType */
	1,			/* bJackID */
	0,			/* iJack */

	6,			/* bLength */
	36,			/* descriptor type */
	2,			/* MIDI_IN_JACK desc subtype */
	2,			/* EXTERNAL bJackType */
	2,			/* bJackID */
	0,			/* iJack */

	//B.4.4 MIDI OUT Jack Descriptor
	9,			/* length of descriptor in bytes */
	36,			/* descriptor type */
	3,			/* MIDI_OUT_JACK descriptor */
	1,			/* EMBEDDED bJackType */
	3,			/* bJackID */
	1,			/* No of input pins */
	2,			/* BaSourceID */
	1,			/* BaSourcePin */
	0,			/* iJack */

	9,			/* bLength of descriptor in bytes */
	36,			/* bDescriptorType */
	3,			/* MIDI_OUT_JACK bDescriptorSubtype */
	2,			/* EXTERNAL bJackType */
	4,			/* bJackID */
	1,			/* bNrInputPins */
	1,			/* baSourceID (0) */
	1,			/* baSourcePin (0) */
	0,			/* iJack */


	// B.5 Bulk OUT Endpoint Descriptors

	//B.5.1 Standard Bulk OUT Endpoint Descriptor
	9,			/* bLenght */
	USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
	0x1,			/* bEndpointAddress OUT endpoint number 1 */
	3,			/* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
	8, 0,			/* wMaxPacketSize */
	10,			/* bIntervall in ms */
	0,			/* bRefresh */
	0,			/* bSyncAddress */

	// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
	5,			/* bLength of descriptor in bytes */
	37,			/* bDescriptorType */
	1,			/* bDescriptorSubtype */
	1,			/* bNumEmbMIDIJack  */
	1,			/* baAssocJackID (0) */


	//B.6 Bulk IN Endpoint Descriptors

	//B.6.1 Standard Bulk IN Endpoint Descriptor
	9,			/* bLenght */
	USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
	0x81,			/* bEndpointAddress IN endpoint number 1 */
	3,			/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	8, 0,			/* wMaxPacketSize */
	10,			/* bIntervall in ms */
	0,			/* bRefresh */
	0,			/* bSyncAddress */

	// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
	5,			/* bLength of descriptor in bytes */
	37,			/* bDescriptorType */
	1,			/* bDescriptorSubtype */
	1,			/* bNumEmbMIDIJack (0) */
	3,			/* baAssocJackID (0) */
};

uchar curInt = 0x00;

uchar statusDev[2] = {0x00, 0x00};
const uchar statusInt[2] = {0x00, 0x00};

uchar usbFunctionDescriptor(usbRequest_t * rq)
{
	if(rq -> wValue.bytes[1] == USBDESCR_DEVICE)
	{
		usbMsgPtr = (int)deviceDescrMIDI;
		return sizeof(deviceDescrMIDI);
	}
	else if(rq -> wValue.bytes[1] == USBDESCR_CONFIG)
	{
		usbMsgPtr = (int)configDescrMIDI;
		return sizeof(configDescrMIDI);
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
	
	// DEBUG LED
	PORTD ^= (1 << LED_D0) ^ (1 << LED_D1);
	return 1;
}

void usbFunctionWriteOut(uchar * data, uchar len)
{
	// DEBUG LED
	PORTD ^= (1 << LED_D0) ^ (1 << LED_D1);
}

void main(void)
{
	volatile int i;
	
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
		
		for (i = 0; i <= 32766; i++) // ~ 3 ms
		{
			
		}
		
		PORTD ^= (1 << LED_D0) ^ (1 << LED_D1);
    }
}

