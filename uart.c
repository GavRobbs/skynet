#include "uart.h"
#include <avr/io.h>

volatile RingBuffer input_buffer = {{0}, 0, 0};
volatile RingBuffer output_buffer = {{0}, 0, 0};
char command[COMMAND_BUFFER_SIZE];

uint8_t rb_addchar(volatile RingBuffer * rb, char c)
{
	uint8_t next = (rb->head + 1) % COMMAND_BUFFER_SIZE;
	if(next == rb->tail){
		return 0;
	}

	rb->buffer[rb->head] = c;
	rb->head = next;

	return 1;
}

uint8_t rb_readchar(volatile RingBuffer *rb, char  *c){

	if(rb->head == rb->tail){
		return 0;
	}

	*c = rb->buffer[rb->tail];
	rb->tail = (rb->tail + 1) % COMMAND_BUFFER_SIZE;
	return 1;
}

uint8_t buffer_isempty(volatile RingBuffer *rb)
{
	return rb->head == rb->tail;
}

uint8_t buffer_isfull(volatile RingBuffer *rb)
{
	uint8_t next = (rb->head + 1) % COMMAND_BUFFER_SIZE;
	return next == rb->tail;
}

void wb_putchar(volatile RingBuffer * wb, char c)
{
	uint8_t next = 0;
	do {
		next = (wb->head + 1) % COMMAND_BUFFER_SIZE;
	} while(next == wb->tail); 
	wb->buffer[wb->head] = c;
	wb->head = next;
	UCSR0B |= (1 << UDRIE0);
}

uint8_t uart_getline(char * out, uint8_t maxlen)
{
	char c;
	static uint8_t index;
	static uint8_t discarding;

	while(rb_readchar(&input_buffer, &c)){

		if (c == '\r') {
    		continue;
		}

		if (discarding) {
            if (c == '\n') {
                discarding = 0;
                index = 0;
            }
            continue;
        }

		if(c == '\n'){
			out[index] = '\0';
			index = 0;
			return 1;
		}

		if(index < maxlen - 1){
			out[index++] = c;
		} else{
			discarding = 1;
			index = 0;
		}

	}
	return 0;
}

void uart_write(const char *s)
{
	while(*s){
		wb_putchar(&output_buffer, *s++);
	}
}

void uart_init()
{
	UBRR0H = (uint8_t)(UBRR_VAL >> 8);
	UBRR0L = (uint8_t)UBRR_VAL;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	sei();
}