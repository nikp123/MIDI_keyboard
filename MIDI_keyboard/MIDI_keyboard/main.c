#include "defines.h"

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

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
	// DEBUG LED:
	PORTD &= ~(1 << LED_D0);
}

void main(void)
{
	const uchar keyCode [8] = 
	{		// PIN-PORT -> "note":
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
	
	uchar adc_ch = 0;
	uchar adc_old[2] = {0x7F, 0x7F};
	uchar adc_temp;
	
	uchar ind_midi_msg = 0; // in one pass of the main cycle - transmit one MIDI msg, index of key which will transmit in MIDI msg
	uchar midiMsg[4];
	
	uchar key[3] = {0xFF, 0xFF, 0xFF,}; // keys status: for "majoritar"
	
	uchar cnt_keys_poll = 0;
	uchar ind_key_poll = 0;
	
	uchar key_old = 0xFF; 
	//uchar key_old = 0xFC; // DEBUG: fast imitate of release key
	
	uchar key_mask;
	uchar key_num_ch[8] = // number of keys that was changed: release (2) or press (1), don't changed (0):
							{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
	
	uchar flag_r_or_p; // release or pressed key flag
	
	PORTB = (1 << BUT_B0) | (1 << BUT_B1) | (1 << BUT_B2) | (1 << BUT_B3);
	//DDRB = ~(1 << BUT_B0) & ~(1 << BUT_B1) & ~(1 << BUT_B2) & ~(1 << BUT_B3);
	
	PORTC = (1 << BUT_C0) | (1 << BUT_C1);
	//DDRC &= ~(1 << BUT_C0) & ~(1 << BUT_C1);
	
	//PORTD &= ~(1 << LED_D0) & ~(1 << LED_D1) & ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT);
	PORTD = (1 << BUT_D0) | (1 << BUT_D1);
	
	DDRD = (1 << LED_D0) | (1 << LED_D1);
	//DDRD &= ~(1 << USB_CFG_DMINUS_BIT) & ~(1 << USB_CFG_DPLUS_BIT) & ~(1 << BUT_D0) & ~(1 << BUT_D1);
	
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // presc 128
	//ADCSRA = (1 << ADEN) | (1 << ADFR) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // continues conversion
	//ADCSRA |= (1 << ADSC); // start conversion if "ADFR" = 0
	
	ADMUX = (1 << ADLAR) | (1 << REFS0); // ref: "AVcc" with ext cap on "AREF" pin, read data only from "ADCH"
	//ADMUX |= (1 << MUX3) | (1 << MUX2) | (1 << MUX1); // apply 1.3 V to ADC input for calibration
	//ADMUX |= (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0); // apply 0 V to ADC input for calibration
	
	usbDeviceConnect();
	usbInit();
    
	sei();
	while (1) 
    {
		usbPoll(); // ~ 154 us
		
		if(cnt_keys_poll == DEBOUNCE)
		{
			key[ind_key_poll] = KEY_STATUS;
			
			if(ind_key_poll < 3) ind_key_poll++;
			else ind_key_poll = 0;
			
			PORTD ^= (1 << LED_D0);
		}
		
		// send MIDI msg if keys was changed:
		if(usbInterruptIsReady())
		{
			if((key[0] == key[1]) && (key[1] == key[2]) && (key[0] != key_old) && (ind_key_poll == 3))
			{
				key_mask = key[0] ^ key_old;
				
				key[0] &= key_mask;
				key_old &= key_mask;
				
				for(i = 0; i <= 7; i++)
				{
					flag_r_or_p = (key_old & 0x01) - (key[0] & 0x01);
					
					if(flag_r_or_p == 0x01) key_num_ch[i] = 1; // key was pressed
					else if(flag_r_or_p == 0xFF) key_num_ch[i] = 2; // released
					
					key[0] >>= 1;
					key_old >>= 1;
				}
				
				key[0] = key[1];
				key_old = key[1];
				ind_midi_msg = 0;
				
				PORTD ^= (1 << LED_D1);
			}
		}
		
		if(key_num_ch[ind_midi_msg] == 1)
		{
			midiMsg[0] = 0x09;
			midiMsg[1] = 0x90;					// channel for "Note on" event: 0b1001_NNNN, NNNN - number of channel
			midiMsg[2] = keyCode[ind_midi_msg];	// code note
			midiMsg[3] = 0x7F;					// velocity
				
			key_num_ch[ind_midi_msg] = 0;
			usbSetInterrupt(midiMsg, 4);
		}
		else if(key_num_ch[ind_midi_msg] == 2)
		{
			midiMsg[0] = 0x08;
			midiMsg[1] = 0x80;					// "Note off" event: 0b1000_NNNN
			midiMsg[2] = keyCode[ind_midi_msg];
			midiMsg[3] = 0x00;
				
			key_num_ch[ind_midi_msg] = 0;
			usbSetInterrupt(midiMsg, 4);
		}
		else // check ADC
		{
			adc_ch ^= 0x01;  
			ADMUX ^= (1 << MUX1); // 0 or 1 ch select
			ADCSRA |= (1 << ADSC);
			
			while(ADCSRA & (1 << ADSC)) { } // wait conversion
			
			if(adc_old[adc_ch] >= ADCH) adc_temp = adc_old[adc_ch] - ADCH;
			else adc_temp = ADCH - adc_old[adc_ch];
			
			if(adc_temp > THRESHOLD)
			{
				midiMsg[0] = 0x0B;
				midiMsg[1] = 0xB0;					// "Control Change" event: 0b1011_NNNN
				midiMsg[2] = adc_ch + 0x46;			// channel number 70...77 "0b0nnnnnnn"
				midiMsg[3] = ADCH >> 1;				// value "0b0vvvvvvv"
				
				adc_old[adc_ch] = ADCH;
				usbSetInterrupt(midiMsg, 4);
			}
		}
		
		//usbSetInterrupt(midiMsg, 4); // ~ 359 us
		
		if(ind_midi_msg < 7) ind_midi_msg++;
		else ind_midi_msg = 0;
		
		if(cnt_keys_poll < DEBOUNCE) cnt_keys_poll++; // one main cycle pass ~ 0.5 ms
		else if(ind_key_poll == 3) cnt_keys_poll = 0;
		
ERROR:	if(curInt == 0x00) 
		{
			PORTD &= ~(1 << LED_D0) & ~(1 << LED_D1); // wrong interface: required reset
			usbPoll();
			goto ERROR;
		}
    }
}

