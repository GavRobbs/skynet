#!/bin/bash

echo "Removing any old files"
rm *.elf *.hex 2> /dev/null

echo "Compiling C files"
avr-gcc -Wall -Wpedantic -O2 -mmcu=atmega328p -DF_CPU=8000000UL -o skynet.elf main.c parser.c print.c uart.c variables.c gpio.c

echo "Linking C files"
avr-objcopy -O ihex skynet.elf skynet.hex

echo "Flashing to AVR with avrdude"
avrdude -c usbtiny -p m328p -U flash:w:skynet.hex:i

echo "Cleaning up"
rm *.elf *.hex 2> /dev/null
