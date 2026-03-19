#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>
#include "uart.h"
#include "print.h"
#include "parser.h"
#include "variables.h"

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
		uart_write("OK\r\n");
	} else if(strcmp(commandString, "PRINT") == 0){
		handle_print(arg1);
	} else if(strcmp(commandString, "ASSIGN") == 0){
		handle_assign(arg1);
	} else{
		uart_write("ERR: UNKNOWN COMMAND\r\n");
	}
}

int main(void)
{
	uart_init();
	vr_initall();
	uart_write("Command interpreter online\r\n");

	while(1){
		if(uart_getline(command, sizeof(command)))
		{
			handle_command(command);
		}

	}
	return 1;
}
