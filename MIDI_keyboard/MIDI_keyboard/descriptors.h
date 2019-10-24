#define UNUSED 0x00

const static int PROGMEM prodStrDesr[] =
{
	USB_STRING_DESCRIPTOR_HEADER(7),
	's', 's', '_', 'm', 'i', 'd', 'i'
};

const static uchar PROGMEM devDesc[] = {
	0x12,					/* sizeof(usbDescriptorDevice): length of descriptor in bytes */
	USBDESCR_DEVICE,		/* descriptor type */
	0x10, 0x01,				/* USB version supported */
	UNUSED,					/* device class: defined at interface level */
	UNUSED,					/* subclass */
	UNUSED,					/* protocol */
	0x08,					/* max packet size */
	USB_CFG_VENDOR_ID,		/* 2 bytes */
	USB_CFG_DEVICE_ID,		/* 2 bytes */
	USB_CFG_DEVICE_VERSION,	/* 2 bytes */
	0x01,					/* manufacturer string index */
	0x02,					/* product string index */
	UNUSED,					/* serial number string index */
	0x01,					/* number of configurations */
};

const static uchar PROGMEM confDesc[] = {
	
				/****************** Configuration Descriptor ******************/
	
	0x09,						/* sizeof(usbDescrConfig): length of descriptor in bytes */
	USBDESCR_CONFIG,			/* descriptor type */
	0x65, 0x00,					/* total length of data returned (including inlined descriptors) */
	0x02,						/* number of interfaces in this configuration */
	0x01,						/* index of this configuration */
	UNUSED,						/* configuration name string index */
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
	UNUSED,					/* interface protocol */
	UNUSED,					/* string index for interface */

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
	UNUSED,					/* interface protocol */
	UNUSED,					/* string index for interface */
		
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
	2,						/* bIntervall in ms */
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
	0x03,					/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	0x08, 0x00,				/* max packet size */
	2,						/* bIntervall in ms */
	UNUSED,					/* bRefresh */
	UNUSED,					/* bSyncAddress */

	// Class-specific MS Bulk IN Endpoint Descriptor:
	0x05,					/* length of descriptor in bytes */
	USBDESCR_CS_ENDPOINT,	/* descriptor type */
	0x01,					/* descriptor subtype: MS_GENERAL */
	0x01,					/* number of embedded MIDI OUT Jacks */
	0x03,					/* id of the Embedded MIDI OUT Jack */
};