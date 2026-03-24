#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <util/delay.h>
#include "uart.h"
#include "print.h"
#include "parser.h"
#include "variables.h"
#include "gpio.h"

ISR(USART_RX_vect){
	char c = UDR0;

	rb_addchar(&input_buffer, c);
}

ISR(USART_UDRE_vect){
	if(output_buffer.head == output_buffer.tail)
	{
		UCSR0B &= ~(1 << UDRIE0);
		return;
	}

	UDR0 = output_buffer.buffer[output_buffer.tail];
	output_buffer.tail = (output_buffer.tail + 1) % COMMAND_BUFFER_SIZE;
}

void handle_command(char * commandString)
{
	char *arg1 = strchr(commandString, ' ');
	if(arg1)
	{
		*arg1 = '\0';
		arg1++;
	}

	if(strcmp(commandString, "STATUS") == 0)
	{
		uart_writeP(PSTR("OK\r\n"));
	} else if(strcmp(commandString, "PRINT") == 0){
		handle_print(arg1);
	} else if(strcmp(commandString, "ASSIGN") == 0){
		handle_assign(arg1);
	} else if(strcmp(commandString, "MODE") == 0){
		handle_setgpio_mode(arg1);
	} else if(strcmp(commandString, "WRITE") == 0){
		handle_gpio_write(arg1);
	} else{
		uart_writeP(PSTR("ERR: UNKNOWN COMMAND\r\n"));
	}
}

int main(void)
{
	uart_init();
	vr_initall();
	uart_writeP(PSTR("Command interpreter online\r\n"));

	while(1){
		if(uart_getline(command, sizeof(command)))
		{
			handle_command(command);
		}

	}
	return 1;
}
