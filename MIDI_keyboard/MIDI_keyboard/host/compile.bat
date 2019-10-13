ECHO OFF
gcc -I D:\SS\source_mc\MIDI_keyboard\MIDI_keyboard\MIDI_keyboard\libusb\include -L D:\SS\source_mc\MIDI_keyboard\MIDI_keyboard\MIDI_keyboard\libusb\lib\gcc -O -Wall main.c -o main.exe -lusb
PAUSE