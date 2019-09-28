#include "defines.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "usbdrv/usbconfig.h"

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	return 0;
}

int main(void)
{
	USB_PORT &= ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT);
	USB_DDR &= ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT);
	
	BUT_PORT = 0xFF;
	BUT_DDR = 0x00;
	
	usbDeviceDisconnect();
	_delay_ms(500);
	usbDeviceConnect();
	
	usbInit();
    
	sei();
	while (1) 
    {
		usbPoll();
		_delay_ms(10);
    }
}

