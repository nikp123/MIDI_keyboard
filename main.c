#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "defines.h"
#include "usbdrv/usbdrv.h"
#include "descriptors.h"

//#define KEY_STATUS ((PINB & 0b00001111) | (PINC & 0b00110000) | (PIND & 0b11000000))
//   key num: 7 6 5 4 3 2 1 0
// PORT name: D D C C B B B B, bind by scheme
/* equivalent to this:		  
   (PINB & ((1 << BUT_B0) | (1 << BUT_B1) | (1 << BUT_B2) | (1 << BUT_B3))) | 
   (PINC & ((1 << BUT_C0) | (1 << BUT_C1))) | 
   (PIND & ((1 << BUT_D0) | (1 << BUT_D1)))	
   */

uint8_t curInt = 0x01;
uint8_t bReq;
uint8_t wInd;

uint8_t statusDev[2] = {0x00, 0x00}; // ???
const uint8_t statusInt[2] = {0x00, 0x00};

uint8_t usbFunctionDescriptor(usbRequest_t * rq)
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

USB_PUBLIC uint8_t usbFunctionSetup(uint8_t data[8])
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

USB_PUBLIC uint8_t usbFunctionRead(uint8_t * data, uint8_t len) // mb turn off in usbconfig ???
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

USB_PUBLIC uint8_t usbFunctionWrite(uint8_t * data, uint8_t len) // mb turn off in usbconfig ???
{
  switch(bReq)
  {
    case USBRQ_SET_INTERFACE: 
      curInt = wInd;
      return 1;
    default: return 0xFF;
  }
}

USB_PUBLIC void usbFunctionWriteOut(uint8_t * data, uint8_t len) // mb turn off in usbconfig ???
{

}

#define col_count 8
#define row_count 8

enum mode {
  OUTPUT_HIGH,
  OUTPUT_LOW,
  INPUT_PULLUP,
  INPUT
};

enum pin {
  IO_PC0 = 0, IO_PC1 = 1, IO_PC2 = 2, IO_PC3 = 3, IO_PC4 = 4, IO_PC5 = 5, IO_PC6 = 6, IO_PC7 = 7,
  IO_PD0 = 8, IO_PD1 = 9, IO_PD2 =10, IO_PD3 =11, IO_PD4 =12, IO_PD5 =13, IO_PD6 =14, IO_PD7 =15,
  IO_PB0 =16, IO_PB1 =17, IO_PB2 =18, IO_PB3 =19, IO_PB4 =20, IO_PB5 =21, IO_PB6 =22, IO_PB7 =23
};


const enum pin cols[col_count] = {
  IO_PC2,
  IO_PC3,
  IO_PC4,
  IO_PC5,
  IO_PD0,
  IO_PD1,
  IO_PD3,
  IO_PD7
};

const enum pin rows[row_count] = {
  IO_PB0,
  IO_PB1,
  IO_PB2,
  IO_PB3,
  IO_PB4,
  IO_PB5,
  IO_PC0,
  IO_PC1
};

uint8_t readPin(enum pin pin) {
  if(pin >= IO_PC0 && pin <= IO_PC7) { // IO_PC
    return (PINC >> (pin - IO_PC0)) & 0x1;
  } else if(pin >= IO_PD0 && pin <= IO_PD7) { // IO_PD
    return (PIND >> (pin - IO_PD0)) & 0x1;
  } else if(pin >= IO_PB0 && pin <= IO_PB7) { // IO_PB
    return (PINB >> (pin - IO_PB0)) & 0x1;
  } else {
    return 0;  
  }
}


void pinMode(enum pin pin, enum mode mode) {
  if(pin > IO_PC0 && pin < IO_PD0) { // IO_PC
    switch(mode) {
      case OUTPUT_LOW:
        DDRC  |=  (1 << (pin - IO_PC0));
        PORTC &= ~(1 << (pin - IO_PC0));
        break;
      case OUTPUT_HIGH:
        DDRC  |=  (1 << (pin - IO_PC0));
        PORTC |=  (1 << (pin - IO_PC0));
        break;
      case INPUT:
        DDRC  &= ~(1 << (pin - IO_PC0));
        PORTC &= ~(1 << (pin - IO_PC0));
        break;
      case INPUT_PULLUP:
        DDRC  &= ~(1 << (pin - IO_PC0));
        PORTC |=  (1 << (pin - IO_PC0));
        break;
    }
  } else if(pin > IO_PD0 && pin <= IO_PD7) { // IO_PD
    switch(mode) {
      case OUTPUT_LOW:
        DDRD  |=  (1 << (pin - IO_PD0));
        PORTC &= ~(1 << (pin - IO_PD0));
        break;
      case OUTPUT_HIGH:
        DDRD  |=  (1 << (pin - IO_PD0));
        PORTC |=  (1 << (pin - IO_PD0));
        break;
      case INPUT:
        DDRD  &= ~(1 << (pin - IO_PD0));
        PORTC &= ~(1 << (pin - IO_PD0));
        break;
      case INPUT_PULLUP:
        DDRD  &= ~(1 << (pin - IO_PD0));
        PORTC |=  (1 << (pin - IO_PD0));
        break;
    }
  }
}

typedef union key_state {
  uint64_t whole;
  uint8_t  byte[8]; // 32 + 5 keys
} key_state;

key_state keyscan() {
  key_state scan;
  // prepare matrix pins (INPUT_PULLUP)
  DDRB &= ~(0b00111111);
  DDRC &= ~(0b00111111);
  DDRD &= ~(0b10001011);
  PORTB |=  0b00111111;
  PORTC |=  0b00111111;
  PORTD |=  0b10001011;
  
  for(uint8_t i = 0; i < col_count; i++) {
    pinMode(cols[i], OUTPUT_LOW);
    scan.byte[i] = 0x00;
    for(uint8_t j = 0; j < row_count; j++) {
      scan.byte[i] |= ((readPin(rows[j]) ? 0 : 1) << j);
    }
    pinMode(cols[i], INPUT_PULLUP);
  }

  return scan;
}

void setup(void);
void loop(void);

int main(void)
{
  setup();
  while(1) {
    loop();
  }
}

void setup(void) {
  // init:
  usbDeviceConnect();
  usbInit();

  sei();
}

// ADC:
#define adc_ch_num 8
uint8_t adc_ch = 0;
int adc_old[adc_ch_num] = { 100 };
int adc_buf[adc_ch_num] = { 100 };
int adc_temp;


// MIDI:	
uint8_t ind_midi_msg = 0; // in one pass of the main cycle - transmit one MIDI msg, index of key which will transmit in MIDI msg
uint8_t midi_msg[4];

// keyboard:
#define KEYBOARD_KEYS 32
key_state key[3] = { { .whole = 0xFFFFFFFFFFFFFFFF } }; // keys status: for "majoritar"
key_state key_old = { .whole = 0xFFFFFFFFFFFFFFFF }; 

// number of keys that was changed: release (2) or press (1), don't changed (0):
uint8_t key_num_ch[KEYBOARD_KEYS] = { 0x00 };

uint8_t cnt_key_poll = 0; // number of polling keyboard for "majoritar" (3 times)

uint16_t cnt_deb = 0;
uint8_t cnt_deb_beet = 0;

void loop(void) {
  usbPoll(); // ~ 9.63 us (all timings write in 16 MHz CPU freq)

  if((cnt_deb == DEB) & (cnt_deb_beet == 0))
  {
    key[cnt_key_poll] = keyscan();
    cnt_key_poll++;
  }

  // DEBOUNCE
  if((key[0].whole == key[1].whole) && 
    (key[1].whole == key[2].whole) &&
    (key[0].whole != key_old.whole) &&
    (cnt_key_poll == 3)) {

    // show which bits changed
    key_state key_mask = { .whole = key[0].whole ^ key_old.whole };

    // fetch changed bits from new and old
    key[0].whole  &= key_mask.whole;
    key_old.whole &= key_mask.whole;

    // handle first 32 keys
    for(uint8_t i = 0; i <= KEYBOARD_KEYS; i++) {
      uint8_t flag_r_or_p = (key_old.whole & 0x01) - (key[0].whole & 0x01);

      if(flag_r_or_p == 0xFF)      key_num_ch[i] = 1; // key was pressed
      else if(flag_r_or_p == 0x01) key_num_ch[i] = 2; // released

      key[0].whole >>= 1;
      key_old.whole >>= 1;
    }

    key[0].whole = key[1].whole;
    key_old.whole = key[1].whole;
    ind_midi_msg = 0;
  }

  // if keys was changed - save num and direction of changed keys:
  if(usbInterruptIsReady())
  {
    // send MIDI msg for changed keys:
    if(key_num_ch[ind_midi_msg] == 1)
    {
      midi_msg[0] = 0x09;
      midi_msg[1] = 0x90;					 // "Note on" event: 0b1001_NNNN, NNNN - number of channel
      midi_msg[2] = 0x3C + ind_midi_msg;
      midi_msg[3] = 0x7F;					 // velocity "0b0vvvvvvv"

      key_num_ch[ind_midi_msg] = 0;
      usbSetInterrupt(midi_msg, 4); // ~ 22.44 us
    }
    else if(key_num_ch[ind_midi_msg] == 2)
    {
      midi_msg[0] = 0x08;
      midi_msg[1] = 0x80;					 // "Note off" event: 0b1000_NNNN
      midi_msg[2] = 0x3C + ind_midi_msg;
      midi_msg[3] = 0x00;

      key_num_ch[ind_midi_msg] = 0;
      usbSetInterrupt(midi_msg, 4); // ~ 22.44 us
    } 
    // else check ADC:
    else {
      //adc_temp = 0;
      //adc_temp = adc_buf[adc_ch];

      //if(((adc_old[adc_ch] - adc_temp) > THRESHOLD) || ((adc_old[adc_ch] - adc_temp) < -THRESHOLD))
      //{
      //  midi_msg[0] = 0x0B;
      //  midi_msg[1] = 0xB0;				// "Control Change" event: 0b1011_NNNN
      //  midi_msg[2] = adc_ch + 0x5A;	// channel number 70...77 "0b0nnnnnnn" (sel 90,91)
      //  midi_msg[3] = adc_temp >> 1;	// value "0b0vvvvvvv"

      //  adc_old[adc_ch] = adc_temp;
      //  usbSetInterrupt(midi_msg, 4);
      //}
    }

  }

  if(ind_midi_msg < KEYBOARD_KEYS) ind_midi_msg++;
  else ind_midi_msg = 0;

  if(cnt_deb < DEB) cnt_deb++; // one main cycle pass ~ 15-16 us
  else if(cnt_key_poll == 3) 
  {
    cnt_deb = 0;
    cnt_key_poll = 0;
  }

  if((cnt_deb == DEB) & (cnt_deb_beet < DEB_BEET)) cnt_deb_beet++;
  else cnt_deb_beet = 0;
}
