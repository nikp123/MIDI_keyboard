#include <avr/io.h>
#include <avr/interrupt.h>

#include "defines.h"
#include "usbdrv/usbdrv.h"
#include "descriptors.h"

#define uint unsigned int
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
		usbMsgPtr = (usbMsgPtr_t)devDesc;
		return sizeof(devDesc);
	}
	else if(rq -> wValue.bytes[1] == USBDESCR_CONFIG)
	{
		usbMsgPtr = (usbMsgPtr_t)confDesc;
		return sizeof(confDesc);
	}
	else if((rq -> wValue.bytes[1] == USBDESCR_STRING) & (rq -> wValue.bytes[0] == 2)) // device name
	{
		usbMsgPtr = (usbMsgPtr_t)prodStrDesr;
		return sizeof(prodStrDesr);
	}
	else return 0xFF;
}

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (usbRequest_t*) data;

	bReq = rq -> bRequest;
	wInd = rq -> wIndex.bytes[0];
	
	switch(bReq)
	{
		case USBRQ_GET_STATUS:
			if((rq -> bmRequestType & USBRQ_RCPT_MASK) == USBRQ_RCPT_DEVICE)
			{
				usbMsgPtr = (usbMsgPtr_t)&statusDev[0];
				return sizeof(statusDev);
			}
			else if((rq -> bmRequestType & USBRQ_RCPT_MASK) == USBRQ_RCPT_INTERFACE)
			{
				if((rq -> wIndex.bytes[0] == 0x00) | (rq -> wIndex.bytes[0] == 0x01))
				{
					usbMsgPtr = (usbMsgPtr_t)&statusInt[0];
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

USB_PUBLIC uchar usbFunctionRead(uchar * data, uchar len) // mb turn off in usbconfig ???
{
	switch(bReq)
	{
		case USBRQ_GET_CONFIGURATION:
			data[0] = 0x01;
			return 1;
		case USBRQ_GET_INTERFACE: // ??? descr "interface" and "endpoint" not addressed individually, they inc in device descr
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

USB_PUBLIC uchar usbFunctionWrite(uchar * data, uchar len) // mb turn off in usbconfig ???
{
	switch(bReq)
	{
		case USBRQ_SET_INTERFACE: 
			curInt = wInd;
			return 1;
		default: return 0xFF;
	}
}

USB_PUBLIC void usbFunctionWriteOut(uchar * data, uchar len) // mb turn off in usbconfig ???
{
	
}

uchar adc_ch = 0;
uchar cnt_adc_smpl = 0;

int adc_buf[2][16];

ISR(ADC_vect)
{
	sei(); // mb not required
	
	if(cnt_adc_smpl < 16) 
	{
		adc_buf[adc_ch][cnt_adc_smpl] = ADCH;
		cnt_adc_smpl++;
	}
}

inline void hardwareInit(void)
{
// IO PORT:
	PORTB = (1 << BUT_B0) | (1 << BUT_B1) | (1 << BUT_B2) | (1 << BUT_B3);
	DDRB = 0; // all input with pull up

	PORTC = (1 << BUT_C0) | (1 << BUT_C1);
	DDRC = 0; // all input with pull up

	PORTD = (1 << BUT_D0) | (1 << BUT_D1);
	DDRD = (1 << LED_D0) | (1 << LED_D1);

// ADC:
	ADCSRA = (1 << ADEN) | (1 << ADFR) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // continues conversion, presc 128

	ADMUX = (1 << ADLAR) | (1 << REFS0); // ref: "AVcc" with ext cap on "AREF" pin, read data only from "ADCH"
	//ADMUX |= (1 << MUX3) | (1 << MUX2) | (1 << MUX1); // apply 1.3 V to ADC input for calibration
	//ADMUX |= (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0); // apply 0 V to ADC input for calibration
}

void main(void)
{
	const uchar key_code [8] = 
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

// ADC:	
	int adc_old[2] = {0, 0};
	int adc_temp;

// MIDI:	
	uchar ind_midi_msg = 0; // in one pass of the main cycle - transmit one MIDI msg, index of key which will transmit in MIDI msg
	uchar midi_msg[4];

// keyboard:	
	uchar key[3] = {0xFF, 0xFF, 0xFF}; // keys status: for "majoritar"
	uchar key_old = 0xFF; 
	
	uchar key_mask;
	uchar key_num_ch[8] = // number of keys that was changed: release (2) or press (1), don't changed (0):
							{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uchar cnt_key_poll = 0; // number of polling keyboard for "majoritar" (3 times)
	uchar flag_r_or_p; // release or pressed key flag
	
	uint  cnt_deb = 0;
	uchar cnt_deb_beet = 0;

// init:	
	hardwareInit();
	usbDeviceConnect();
	usbInit();
    
	ADCSRA |= (1 << ADSC); // start ADC
	sei();
	while (1) 
    {
		usbPoll(); // ~ 9.63 us (all timings write in 16 MHz CPU freq)
		
		if((cnt_deb == DEB) & (cnt_deb_beet == 0))
		{
			key[cnt_key_poll] = KEY_STATUS;
			cnt_key_poll++;
		}
		
		// if keys was changed - save num and direction of changed keys:
		if(usbInterruptIsReady())
		{
			if((key[0] == key[1]) && (key[1] == key[2]) && (key[0] != key_old) && (cnt_key_poll == 3))
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
				
				LED1_TOGGLE;
			}
		}
		
		// send MIDI msg for changed keys:
		if(key_num_ch[ind_midi_msg] == 1)
		{
			midi_msg[0] = 0x09;
			midi_msg[1] = 0x90;					 // "Note on" event: 0b1001_NNNN, NNNN - number of channel
			midi_msg[2] = key_code[ind_midi_msg]; // code of note
			midi_msg[3] = 0x7F;					 // velocity "0b0vvvvvvv"
				
			key_num_ch[ind_midi_msg] = 0;
			usbSetInterrupt(midi_msg, 4); // ~ 22.44 us
		}
		else if(key_num_ch[ind_midi_msg] == 2)
		{
			midi_msg[0] = 0x08;
			midi_msg[1] = 0x80;					 // "Note off" event: 0b1000_NNNN
			midi_msg[2] = key_code[ind_midi_msg];
			midi_msg[3] = 0x00;
				
			key_num_ch[ind_midi_msg] = 0;
			usbSetInterrupt(midi_msg, 4);
		} 
		// else check ADC:
		else if(cnt_adc_smpl == 16)
		{
			ADCSRA &= ~(1 << ADIE); // dsbl interrupt for normal filtering and send msg
			
			cnt_adc_smpl = 0;
			adc_temp = 0;
			
			for(i = 0; i <= 15; i++) adc_temp += adc_buf[adc_ch][i];
			adc_temp >>= 4;
			
				LED0_OFF; // activate led only when potentiometer is rotating
			if(((adc_old[adc_ch] - adc_temp) > THRESHOLD) || ((adc_old[adc_ch] - adc_temp) < -THRESHOLD))
			{
				LED0_ON;
				
				midi_msg[0] = 0x0B;
				midi_msg[1] = 0xB0;				// "Control Change" event: 0b1011_NNNN
				midi_msg[2] = adc_ch + 0x5A;	// channel number 70...77 "0b0nnnnnnn" (sel 90,91)
				midi_msg[3] = adc_temp >> 1;	// value "0b0vvvvvvv"
				
				adc_old[adc_ch] = adc_temp;
				usbSetInterrupt(midi_msg, 4);
			}
			
			adc_ch ^= 0x01;
			ADMUX ^= (1 << MUX0); // 0 or 1 ch select
			
			if(ADCSRA & (1 << ADIF)) ADCSRA |= (1 << ADIF); // dsbl flag interrupt because channel not actuality
			ADCSRA |= (1 << ADIE);
		}
		
		if(ind_midi_msg < 7) ind_midi_msg++;
		else ind_midi_msg = 0;
		
		if(cnt_deb < DEB) cnt_deb++; // one main cycle pass ~ 15-16 us
		else if(cnt_key_poll == 3) 
		{
			cnt_deb = 0;
			cnt_key_poll = 0;
		}
		
		if((cnt_deb == DEB) & (cnt_deb_beet < DEB_BEET)) cnt_deb_beet++;
		else cnt_deb_beet = 0; 

/*	
ERROR:	if(curInt == 0x00) 
		{
			LED_OFF; // wrong interface: required reset
			usbPoll();
			goto ERROR;
		}
*/
    }
}

