#ifndef COMMAND_BUFFER_SIZE
#define COMMAND_BUFFER_SIZE 255
#endif

#ifndef F_CPU 
#define F_CPU 8000000UL
#endif

#ifndef BAUD
#define BAUD 9600
#endif

#ifndef UBRR_VAL
#define UBRR_VAL ((F_CPU / 16UL / BAUD) - 1UL)
#endif

#ifndef SN_UART_H
#define SN_UART_H

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct{
	char buffer[COMMAND_BUFFER_SIZE];
	uint8_t head; //This is where we write
	uint8_t tail; //This is where we read from
} RingBuffer;

extern volatile RingBuffer input_buffer;
extern volatile RingBuffer output_buffer;
extern char command[COMMAND_BUFFER_SIZE];

uint8_t rb_addchar(volatile RingBuffer * rb, char c);
uint8_t rb_readchar(volatile RingBuffer *rb, char  *c);
uint8_t buffer_isempty(volatile RingBuffer *rb);
uint8_t buffer_isfull(volatile RingBuffer *rb);
void wb_putchar(volatile RingBuffer * wb, char c);
uint8_t uart_getline(char * out, uint8_t maxlen);
void uart_init();
void uart_write(const char * str);

#endif
