## DIY MIDI keyboard for record drum kit with DAW

MIDI keyboard based on ATMega8(A) (88) using V-USB for connect to PC without special driver. USB descriptor is basic MIDI driver: ```Audio Control Interface Descriptor``` , ```MIDIStreaming Interface Descriptors ```. On ADC input there are 2 potentiometers for any using in DAW (e.g. Volume of current channel and master).

#### Defines:

* Frequency CPU 12 or 16 MHz
* In defines can be set pinout of buttons
* Value of debounce, threshold, etc.
* For now buttons can't transmit value of velocity and MIDI packet has only event of press current button

Improvements: 

* Using another technique of debounce button
* Using velocity
* Increase the number of channels

