## Basic MIDI keyboard using an ATMega8 microcontroller

This is a three octave keyboard which is decoded through a matrix.
Not all keys are mapped yet, I just wanted basic functionality.

## Changes over senya1997 implementation
 - Forced 12MHz clock but can be changed through the definition of F_CPU
 - Used Makefile instead of Visual Studio project file
 - Updated VUSB version so it actaully compiles on modern systems
 - Better USB handling (less chance for bugged MIDI keys)
 - Less customizable but more powerful (able to decode a keyboard matrix)
 - Simplified implementation (a tiny bit)

