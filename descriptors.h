#include <avr/pgmspace.h>

const int PROGMEM prodStrDesr[] =
{
	USB_STRING_DESCRIPTOR_HEADER(7),
	's', 's', '_', 'm', 'i', 'd', 'i'
};

const uchar PROGMEM devDesc[] = { // all desc static ???
	0x12,					/* sizeof(usbDescriptorDevice): length of descriptor in bytes */
	USBDESCR_DEVICE,		/* descriptor type */
	0x10, 0x01,				/* USB version supported */
	UNUSED,					/* device class: defined at interface level (if bDeviceClass = 0x00) */
	UNUSED,					/* subclass */
	UNUSED,					/* protocol: defined at interface level */
	0x08,					/* max packet size (for EP0) */
	USB_CFG_VENDOR_ID,		/* 2 bytes */
	USB_CFG_DEVICE_ID,		/* 2 bytes */
	USB_CFG_DEVICE_VERSION,	/* 2 bytes */
	0x01,					/* manufacturer string index */
	0x02,					/* product string index */
	UNUSED,					/* serial number string index */
	0x01,					/* number of configurations */
};

const uchar PROGMEM confDesc[] = {
	
				/****************** Configuration Descriptor ******************/
	
	0x09,						/* sizeof(usbDescrConfig): length of descriptor in bytes */
	USBDESCR_CONFIG,			/* descriptor type */
	TOTAL_LEN_DESCR, 0x00,		/* total length of data returned (including inlined descriptors) */
	0x02,						/* number of interfaces in this configuration */
	0x01,						/* index of this configuration (??? or value that choose this configuration) */
	UNUSED,						/* configuration name string index */
	USBATTR_BUSPOWER,			/* attributes */
	USB_CFG_MAX_BUS_POWER / 2,	/* max USB current in 2mA units */
	
				/************** AudioControl Interface Descriptor **************/
	
	// Standard AC Interface Descriptor:
	0x09,					/* sizeof(usbDescrInterface): length of descriptor in bytes */
	USBDESCR_INTERFACE,		/* descriptor type */
	0x00,					/* index of this interface */
	UNUSED,					/* alternate setting for this interface (mb used for define alternative interface) */
	0x00,					/* amount endpoints that this interface is use, excl 0 */
	USB_CFG_INTERFACE_CLASS,/* interface class: AUDIO */
	0x01,					/* interface subclass: AUDIO_CONTROL */
	UNUSED,					/* interface protocol (e.g. 0 = none, 1 = keyboard, 2 = mouse for HID) */
	UNUSED,					/* index of string descriptor for this interface */

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
	UNUSED,					/* alternate setting for this interface */
	0x02,					/* amount endpoints that this interface is use, excl 0 */
	USB_CFG_INTERFACE_CLASS,/* interface class: AUDIO */
	USB_CFG_INTERFACE_SUBCLASS,
	USB_CFG_INTERFACE_PROTOCOL,
	UNUSED,					/* index of string descriptor for this interface */
		
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
	UNUSED,					/* iJack */

	// MIDI Adapter MIDI IN Jack Descriptor (External):
	0x06,					/* size of this descriptor */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x02,					/* descriptor subtype: MIDI_IN_JACK */
	0x02,					/* jackType: EXTERNAL */
	0x02,					/* id of this jack */
	UNUSED,					/* iJack */

	// MIDI OUT Jack Descriptor:
	0x09,					/* length of descriptor in bytes */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x03,					/* descriptor subtype: MIDI_OUT_JACK */
	0x01,					/* jackType: EMBEDDED */
	0x03,					/* id of this jack */
	0x01,					/* number of Input Pins of this Jack */
	0x02,					/* id of the Entity to which this Pin is connected: BaSourceID */
	0x01,					/* output Pin number of the Entity to which this Input Pin is connected: BaSourcePin */
	UNUSED,					/* iJack */

	// MIDI Adapter MIDI OUT Jack Descriptor (External):
	0x09,					/* length of descriptor in bytes */
	USBDESCR_CS_INTERFACE,	/* descriptor type */
	0x03,					/* descriptor subtype: MIDI_OUT_JACK */
	0x02,					/* jackType: EXTERNAL */
	0x04,					/* id of this jack */
	0x01,					/* number of Input Pins of this Jack */
	0x01,					/* id of the Entity to which this Pin is connected: BaSourceID */
	0x01,					/* output Pin number of the Entity to which this Input Pin is connected: BaSourcePin */
	UNUSED,					/* iJack */

				/************* Bulk OUT Endpoint Descriptors *************/

	// Standard Bulk OUT Endpoint Descriptor:
	0x09,					/* size of this descriptor */
	USBDESCR_ENDPOINT,		/* descriptor type */
	0x01,					/* endpoint address: OUT endpoint number 1 */
	0x03,					/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	0x08, 0x00,				/* max packet size */
	USB_CFG_INTR_POLL_INTERVAL,
	UNUSED,					/* bRefresh */
	UNUSED,					/* bSyncAddress */

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
	0x03,					/* bmAttributes: 0: Control, 1: Isochronous 2: Bulk, 3: Interrupt endpoint */
	0x08, 0x00,				/* max packet size */
	USB_CFG_INTR_POLL_INTERVAL,
	UNUSED,					/* bRefresh */
	UNUSED,					/* bSyncAddress */

	// Class-specific MS Bulk IN Endpoint Descriptor:
	0x05,					/* length of descriptor in bytes */
	USBDESCR_CS_ENDPOINT,	/* descriptor type */
	0x01,					/* descriptor subtype: MS_GENERAL */
	0x01,					/* number of embedded MIDI OUT Jacks */
	0x03,					/* id of the Embedded MIDI OUT Jack */
};