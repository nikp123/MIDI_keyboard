#include "defines.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "usbdrv/usbconfig.h"

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	if(data[1] == USB_LED_ON) PORTD |= (1 << LED_D0) | (1 << LED_D1);
	else PORTD &= ~(1 << LED_D0) & ~(1 << LED_D1);
	
	return 0;
}

void main(void)
{
	PORTB |= (1 << BUT_B0) | (1 << BUT_B1) | (1 << BUT_B2) | (1 << BUT_B3);
	DDRB &= ~(1 << BUT_B0) & ~(1 << BUT_B1) & ~(1 << BUT_B2) & ~(1 << BUT_B3);
	
	PORTC |= (1 << BUT_C0) | (1 << BUT_C1);
	DDRC &= ~(1 << BUT_C0) & ~(1 << BUT_C1);
	
	PORTD &= ~(1 << LED_D0) & ~(1 << LED_D1) & ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT);
	PORTD |= (1 << BUT_D0) | (1 << BUT_D1);
	
	DDRD |= (1 << LED_D0) | (1 << LED_D1);
	DDRD &= ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT) & ~(1 << BUT_D0) & ~(1 << BUT_D1);	
	
	//usbDeviceDisconnect();
	//_delay_ms(100);
	usbDeviceConnect();
	
	usbInit();
    
	sei();
	while (1) 
    {
		usbPoll();
		_delay_ms(10);
		
    }
}

