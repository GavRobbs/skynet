# Skynet

## A Simple Command Language For AVR Microcontrollers

***

### Introduction

Skynet is a text-based BASIC-like command language for AVR microcontrollers. Its primary design goal is to allow LLMs to reprogram AVR microcontrollers via their peripherals (eg. UART, SPI, I2C) instead of having to flash the chip with a programmer every time. This should in theory allow for autonomous dynamic self-reprogramming of microcontrollers eg. in robotics.

### Syntax

Skynet's syntax is similar to that of BASIC, but with a few modern additions. At the moment, only mathematics on integral types are supported, but this may change in the future. Commands are specified with the command name, followed by a space, followed by the required parameters as a comma separated list.

[COMMAND] [EXPRESSION 1] , [EXPRESSION 2] etc..


### Supported Commands

- ASSIGN [VARIABLE_NAME] = [EXPRESSION]

This command is used to create variables before they are used (which is mandatory). A Skynet instance can have up to 12 variables. Variables are stored as 16 bit integers. Variable names can be up to 8 characters long, and can only start with letters.

Strings are not assignable to variables (since they're mostly for debugging output and returning values). 

#### Examples

ASSIGN X = 5

- PRINT [STRING/EXPRESSIONn]

Prints multiple expressions or strings specified as a comma separated list.

#### Examples

Input: PRINT "HELLO WORLD!"
Output: HELLO WORLD!

Input: PRINT "WELCOME", " TO", "JAMROCK" 
Output: WELCOME TOJAMROCK!

Input: PRINT X = 5
Output: 5

Input: PRINT "When you double X, you get: ", X * 2, "! Isn't that nifty?"
Output: When you double X, you get 10! Isn't that nifty?

- STATUS

Returns the system status. Usually "OK".

- MODE [PORT], [PIN#], [PINMODE]

This sets a GPIO pin to input or output. PORT can be B, C or D (unquoted), PIN# can be from 0-7 and PINMODE is either OUTPUT, INPUT OR INPUT_PULLUP (unquoted)

#### Examples

MODE B, 3, OUTPUT
MODE C, 2, INPUT_PULLUP

- WRITE [PORT], [PIN#], [VALUE]

Writes a 0 or 1 to a specific GPIO pin. PORT can be B, C or D (unquoted), PIN# can be from 0-7. VALUE accepts any valid expression as long as it resolves to 0 or 1.

#### Examples

WRITE D, 4, 1
WRITE B, 7, 0



### Next steps

The next steps in decreasing order of importance are:
- Basic GPIO control commands
- Built in math and bit manipulation functions: POW, HBYTE, LBYTE, SETB, TOGGLEB, CLEARB, AND, OR, XOR, NOT
- Conditions and JUMP statements
- Immediate mode vs program-mode
- PWM and interrupts
- Examples