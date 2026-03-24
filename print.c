#include "print.h"
#include "uart.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>

void handle_print(const char * values)
{
	char output_string[COMMAND_BUFFER_SIZE] = {0};
	uint8_t output_pointer = 0;
	uint8_t input_pointer = 0;

	char inputValue[60] = {0};
	uint8_t iv_pointer = 0;

	enum ParserState {EXPECT_FIRST_ITEM, EXPECT_NEXT_ITEM, READING_STRING, READING_EXPRESSION, EXPECT_SEPARATOR} state = EXPECT_FIRST_ITEM;

	while(values[input_pointer] != '\0' && output_pointer < COMMAND_BUFFER_SIZE - 1)
	{
		char c = values[input_pointer];

		switch(state)
		{
			case EXPECT_FIRST_ITEM:
				if(c == '"')
				{
					state = READING_STRING;
				} else if((c != ' ') && (c != ','))
				{
					state = READING_EXPRESSION;
					inputValue[iv_pointer++] = c;
				} else if((c == ',') || (c == ' '))
				{
					uart_writeP(PSTR("ERR: EXPECTED VALUE OR STRING\r\n"));
					return;
				}
				break;

			case READING_STRING:
				if(c == '"')
				{
					state = EXPECT_SEPARATOR;
				} else{
					output_string[output_pointer++] = c;
				}
				break;

			case READING_EXPRESSION:
				if(c == ',')
				{
					inputValue[iv_pointer] = '\0';
					uint8_t rv = _print_resolve_value(inputValue, output_string, output_pointer);

                    if(rv == 0)
                    {
                        //An error occurred during expression resolution, we should stop processing
                        return;
                    }
					output_pointer += rv;
					iv_pointer = 0;
					state = EXPECT_NEXT_ITEM;

				} else{
					if(iv_pointer < sizeof(inputValue) - 1)
					{
						inputValue[iv_pointer++] = c;
					} else{
						uart_writeP(PSTR("ERR: EXPRESSION TOO LONG\r\n"));
						return;
					}
				}
				break;

			case EXPECT_SEPARATOR:
				if(c == ',')
				{
					state = EXPECT_NEXT_ITEM;
				} else if(c == ' ')
				{
					//Ignore spaces
				} else{
					uart_writeP(PSTR("ERR: UNEXPECTED CHARACTER, EXPECTED SEPARATOR\r\n"));
					return;
				}
				break;

			case EXPECT_NEXT_ITEM:
				if(c == '"')
				{
					state = READING_STRING;
				} else if((c != ' ') && (c != ','))
				{
					state = READING_EXPRESSION;
					inputValue[iv_pointer++] = c;
				} else if(c == ' '){
					//Ignore spaces
				} else if(c == ',')
				{
					uart_writeP(PSTR("ERR: EXPECTED VALUE OR STRING, INAPPROPRIATE SEPARATOR\r\n"));
					return;
				}
				break;
		}

		input_pointer++;
	}

	if(state == READING_EXPRESSION)
	{
		inputValue[iv_pointer] = '\0';
        uint8_t rv = _print_resolve_value(inputValue, output_string, output_pointer);

        if(rv == 0)
        {
            //An error occurred during expression resolution, we should stop processing
            return;
        }

        output_pointer += rv;
	} 

	if(state == READING_STRING)
	{
		//Throw an error for unterminated string or value?
		uart_writeP(PSTR("ERR: UNTERMINATED STRING\r\n"));
		return;
	}

	if(state == EXPECT_NEXT_ITEM)
	{
		//Throw an error for trailing separator?
		uart_writeP(PSTR("ERR: TRAILING SEPARATOR\r\n"));
		return;
	}

	output_string[output_pointer] = '\0';

	uart_write(output_string);

}

uint8_t _print_resolve_value(char * value, char * out_string, uint8_t current_pointer)
{
	//Resolves a variable name or a number and adds it to the out_string, returns
	//how much we should advance the output pointer by

    int16_t result = 0;
    uint8_t status = 0;

    char conversion[8];

    parseExpression(value, strlen(value), 0, &result, &status);

    if(status != 0)
    {
    	uart_writeP(PSTR("ERR: INVALID EXPRESSION\r\n"));
    	return 0;
    }

    itoa(result, conversion, 10);  
    uint8_t i = 0;
    while(conversion[i] != '\0' && current_pointer < COMMAND_BUFFER_SIZE - 1)
    {
    	out_string[current_pointer++] = conversion[i++];
    }

	return i;
}
