#include "gpio.h"
#include <stdint.h>
#include "uart.h"
#include "parser.h"
#include <string.h>
#include <avr/io.h>
#include "variables.h"
#include <avr/pgmspace.h>

void handle_setgpio_mode(const char * line)
{
    /* This is how the GPIO mode setting function is implemented. The line has the following signature:
    MODE PORT, PIN#, MODE_SETTING

    PORT = B, C or D
    PIN# = 0-7
    MODE_SETTING = INPUT, OUTPUT OR INPUT_PULLUP

    */

    enum GPIOParserState{READ_PORT, READ_PIN, READ_MODE_SETTING, EXPECT_SEPARATOR, EXPECT_NEXT_ITEM, DONE}; 
    enum GPIOParserState parserState = READ_PORT;
    enum GPIOParserState nextPhase = READ_PIN;
    char c = ' ';
    char port = ' ';

    uint8_t input_pointer = 0;

    char pinExpression[32] = {0};
    uint8_t pe_pointer = 0;

    char modeLine[16] = {0};
    uint8_t modeline_pointer = 0;

    uint8_t pin = 0;

    while(line[input_pointer] != '\0')
	{
        c = line[input_pointer];

        switch(parserState)
        {
            case EXPECT_SEPARATOR:
                if(c == ' '){
                    //Ignore whitespace when expecting a separator
                } else if(c == ',') {
                    //If we get the separator, move to the next phase
                    parserState = EXPECT_NEXT_ITEM;
                } else{
                    uart_writeP(PSTR("ERR: UNEXPECTED CHARACTER, EXPECTED SEPARATOR \r\n"));
                    return;
                }
                break;
            case READ_PORT:
                if(c == ' '){
                    //Skip any spaces here
                }
                else if(c == 'B' || c == 'C' || c == 'D'){
                    port = c;
                    parserState = EXPECT_SEPARATOR;
                    nextPhase = READ_PIN;                    
                } else {
                    uart_writeP(PSTR("ERR: EXPECTED STRING B, C OR D FOR PORT\r\n"));
                    return;
                }
                break;
            case EXPECT_NEXT_ITEM:
                if(c == ' '){
                    //Consume spaces until something else
                } else if(c == ',') {
                    uart_writeP(PSTR("ERR: EMPTY PARAMETER SPECIFICATION \r\n"));
                    return;
                } else if(nextPhase == READ_PIN){

                    pinExpression[pe_pointer++] = c;
                    parserState = READ_PIN;
                    nextPhase = READ_MODE_SETTING;

                } else if(nextPhase == READ_MODE_SETTING){

                    modeLine[modeline_pointer++] = c;
                    parserState = READ_MODE_SETTING;
                    nextPhase = DONE;

                } else{
                    uart_writeP(PSTR("ERR: DID NOT GET EXPECTED NEXT ITEM \r\n"));
                    return;
                }
                break;
            case READ_PIN:
                if(c == ','){
                    parserState = EXPECT_NEXT_ITEM;
                    nextPhase = READ_MODE_SETTING;
                } else{
                    pinExpression[pe_pointer++] = c;

                    if(pe_pointer == sizeof(pinExpression) - 1){
                        uart_writeP(PSTR("ERR: EXPRESSION FOR PIN IS TOO LONG \r\n"));
                        return;
                    }
                }
                break;
            case READ_MODE_SETTING:
                if(c == ' '){
                    parserState = DONE;
                    break;
                }
                modeLine[modeline_pointer++] = c;

                if(modeline_pointer == sizeof(modeLine) - 1){
                    uart_writeP(PSTR("ERR: INVALID PIN MODE \r\n"));
                    return;
                }
                break;
            case DONE:
                if(c == ' '){
                    //Strip spaces
                } else{
                    uart_writeP(PSTR("ERR: UNEXPECTED CHARACTERS AT END OF EXPRESSION \r\n"));
                    return;
                }
                break;
            default:
                uart_writeP(PSTR("ERR: UNDEFINED STATE \r\n"));
                return;
        }

        input_pointer++;
    }

    if(modeline_pointer == 0){
        uart_writeP(PSTR("ERR: MISSING MODE PARAMETER \r\n"));
        return;
    }

    pinExpression[pe_pointer] = '\0';
    modeLine[modeline_pointer] = '\0';

    int16_t pinRaw = 0;
    uint8_t status = 0;

    parseExpression(pinExpression, strlen(pinExpression), 0, &pinRaw, &status);

    if(status != 0){
        uart_writeP(PSTR("ERR: ERROR EVALUATING PIN EXPRESSION \r\n"));
        return;
    }

    if(pinRaw < 0 || pinRaw > 7){
        uart_writeP(PSTR("ERR: PIN VALUE MUST BE BETWEEN 0 AND 7 INCLUSIVE \r\n"));
        return;
    }

    pin = (uint8_t)pinRaw;

    volatile uint8_t * ddr;
    volatile uint8_t * portr;

    if(port == 'B'){
        ddr = &DDRB;
        portr = &PORTB;
    } else if(port == 'C'){
        ddr = &DDRC;
        portr = &PORTC;
    } else {
        ddr = &DDRD;
        portr = &PORTD;
    }

    if(strcmp(modeLine, "INPUT") == 0){

        *ddr &= ~(1 << pin);
        *portr &= ~(1 << pin);

    } else if(strcmp(modeLine, "OUTPUT") == 0){

        *ddr |= (1 << pin);

    } else if(strcmp(modeLine, "INPUT_PULLUP") == 0){

        *ddr &= ~(1 << pin);
        *portr |= (1 << pin);

    } else{
        uart_writeP(PSTR("ERR: MODE MUST BE INPUT, OUTPUT OR INPUT_PULLUP \r\n"));
        return;
    }

}

void handle_gpio_write(const char * line)
{
    /* 
    
        Writes a high or low value to a gpio pin

        WRITE [PORT] [PIN#] [VALUE]
        PORT and PIN# are as above, VALUE can be 0 or 1

    */

    enum GPIOParserState{READ_PORT, READ_PIN, READ_VALUE, EXPECT_SEPARATOR, EXPECT_NEXT_ITEM, DONE}; 
    enum GPIOParserState parserState = READ_PORT;
    enum GPIOParserState nextPhase = READ_PIN;
    char c = ' ';
    char port = ' ';

    uint8_t input_pointer = 0;

    char pinExpression[32] = {0};
    uint8_t pe_pointer = 0;

    char valueExpression[32] = {0};
    uint8_t value_pointer = 0;

    uint8_t pin = 0;
    uint8_t value = 0;

    while(line[input_pointer] != '\0')
	{
        c = line[input_pointer];

        switch(parserState)
        {
            case EXPECT_SEPARATOR:
                if(c == ' '){
                    //Ignore whitespace when expecting a separator
                } else if(c == ',') {
                    //If we get the separator, move to the next phase
                    parserState = EXPECT_NEXT_ITEM;
                } else{
                    uart_writeP(PSTR("ERR: UNEXPECTED CHARACTER, EXPECTED SEPARATOR \r\n"));
                    return;
                }
                break;
            case READ_PORT:
                if(c == ' '){
                    //Skip any spaces here
                }
                else if(c == 'B' || c == 'C' || c == 'D'){
                    port = c;
                    parserState = EXPECT_SEPARATOR;
                    nextPhase = READ_PIN;                    
                } else {
                    uart_writeP(PSTR("ERR: EXPECTED STRING B, C OR D FOR PORT\r\n"));
                    return;
                }
                break;
            case EXPECT_NEXT_ITEM:
                if(c == ' '){
                    //Consume spaces until something else
                } else if(c == ',') {
                    uart_writeP(PSTR("ERR: EMPTY PARAMETER SPECIFICATION \r\n"));
                    return;
                } else if(nextPhase == READ_PIN){

                    pinExpression[pe_pointer++] = c;
                    parserState = READ_PIN;
                    nextPhase = READ_VALUE;

                } else if(nextPhase == READ_VALUE){

                    valueExpression[value_pointer++] = c;
                    parserState = READ_VALUE;
                    nextPhase = DONE;

                } else{
                    uart_writeP(PSTR("ERR: DID NOT GET EXPECTED NEXT ITEM \r\n"));
                    return;
                }
                break;
            case READ_PIN:
                if(c == ','){
                    parserState = EXPECT_NEXT_ITEM;
                    nextPhase = READ_VALUE;
                } else{
                    pinExpression[pe_pointer++] = c;

                    if(pe_pointer == sizeof(pinExpression) - 1){
                        uart_writeP(PSTR("ERR: EXPRESSION FOR PIN IS TOO LONG \r\n"));
                        return;
                    }
                }
                break;
            case READ_VALUE:
                if(c == ','){
                    uart_writeP(PSTR("ERR: TOO MANY PARAMETERS \r\n"));
                    return;
                } else{
                    valueExpression[value_pointer++] = c;

                    if(value_pointer == sizeof(valueExpression) - 1){
                        uart_writeP(PSTR("ERR: EXPRESSION FOR VALUE IS TOO LONG \r\n"));
                        return;
                    }
                }
                break;
            case DONE:
                if(c == ' '){
                    //Strip spaces
                } else{
                    uart_writeP(PSTR("ERR: UNEXPECTED CHARACTERS AT END OF EXPRESSION \r\n"));
                    return;
                }
                break;
            default:
                uart_writeP(PSTR("ERR: UNDEFINED STATE \r\n"));
                return;
        }

        input_pointer++;
    }

    if(value_pointer == 0){
        uart_writeP(PSTR("ERR: MISSING VALUE PARAMETER \r\n"));
        return;
    }

    if(pe_pointer == 0){
        uart_writeP(PSTR("ERR: MISSING PIN PARAMETER \r\n"));
        return;
    }

    pinExpression[pe_pointer] = '\0';
    valueExpression[value_pointer] = '\0';

    int16_t pinRaw = 0;
    uint8_t pstatus = 0;

    int16_t valRaw = 0;
    uint8_t vstatus = 0;

    parseExpression(pinExpression, pe_pointer, 0, &pinRaw, &pstatus);
    parseExpression(valueExpression, value_pointer, 0, &valRaw, &vstatus);

    if(pstatus != 0){
        uart_writeP(PSTR("ERR: ERROR EVALUATING PIN EXPRESSION \r\n"));
        return;
    }

    if(vstatus != 0){
        uart_writeP(PSTR("ERR: ERROR EVALUATING VALUE EXPRESSION \r\n"));
        return;
    }

    if(pinRaw < 0 || pinRaw > 7){
        uart_writeP(PSTR("ERR: PIN VALUE MUST BE BETWEEN 0 AND 7 INCLUSIVE \r\n"));
        return;
    }

    pin = (uint8_t)pinRaw;

    if(valRaw != 0 && valRaw != 1){
        uart_writeP(PSTR("ERR: VALUE MUST EVALUATE TO 0 OR 1 \r\n"));
        return;
    }

    value = (uint8_t)valRaw;

    volatile uint8_t * portr;

    if(port == 'B'){
        portr = &PORTB;
    } else if(port == 'C'){
        portr = &PORTC;
    } else {
        portr = &PORTD;
    }

    if(value == 0){
        *portr &= ~(1 << pin);
    } else{
        *portr |= (1 << pin);
    }

}

void handle_gpio_read(const char * line)
{
    /* 
    
        Read if a pin has a high or low value and store it in a variable

        READ [VARIABLE] [PORT] [PIN#]
        
        The variable is created if it does not already exist, and PORT and PIN# are as above

    */

    enum GPIOParserState{START, READ_VARIABLE, READ_PORT, READ_PIN, EXPECT_SEPARATOR, EXPECT_NEXT_ITEM, DONE}; 
    enum GPIOParserState parserState = START;
    enum GPIOParserState nextPhase = READ_PORT;
    char c = ' ';
    char port = ' ';

    uint8_t input_pointer = 0;

    char pinExpression[32] = {0};
    uint8_t pe_pointer = 0;

    char vname[9] = {0};
    uint8_t vn_pointer = 0;

    uint8_t pin = 0;
    uint8_t value = 0;

    while(line[input_pointer] != '\0')
	{
        c = line[input_pointer];

        switch(parserState)
        {
            case START:
                if(c == ' '){
                } else{
                    vname[vn_pointer++] = c;
                    parserState = READ_VARIABLE;
                }
                break;
            case EXPECT_SEPARATOR:
                if(c == ' '){
                    //Ignore whitespace when expecting a separator
                } else if(c == ',') {
                    //If we get the separator, move to the next phase
                    parserState = EXPECT_NEXT_ITEM;
                } else{
                    uart_writeP(PSTR("ERR: UNEXPECTED CHARACTER, EXPECTED SEPARATOR \r\n"));
                    return;
                }
                break;
            case READ_PORT:
                if(c == ' '){
                    //Skip any spaces here
                }
                else if(c == 'B' || c == 'C' || c == 'D'){
                    port = c;
                    parserState = EXPECT_SEPARATOR;
                    nextPhase = READ_PIN;                    
                } else {
                    uart_writeP(PSTR("ERR: EXPECTED STRING B, C OR D FOR PORT\r\n"));
                    return;
                }
                break;
            case EXPECT_NEXT_ITEM:
                if(c == ' '){
                    //Consume spaces until something else
                } else if(c == ',') {
                    uart_writeP(PSTR("ERR: EMPTY PARAMETER SPECIFICATION \r\n"));
                    return;
                } else if(nextPhase == READ_PIN){

                    pinExpression[pe_pointer++] = c;
                    parserState = READ_PIN;
                    nextPhase = DONE;

                } else if(nextPhase == READ_PORT){

                    parserState = READ_PORT;
                    nextPhase = READ_PIN;
                    input_pointer--; //We need to reconsume this character in the next state since it's part of the port specification

                } else{
                    uart_writeP(PSTR("ERR: DID NOT GET EXPECTED NEXT ITEM \r\n"));
                    return;
                }
                break;
            case READ_PIN:
                if(c == ','){
                    uart_writeP(PSTR("ERR: TOO MANY PARAMETERS \r\n"));
                    return;
                } else{
                    pinExpression[pe_pointer++] = c;

                    if(pe_pointer == sizeof(pinExpression) - 1){
                        uart_writeP(PSTR("ERR: EXPRESSION FOR PIN IS TOO LONG \r\n"));
                        return;
                    }
                }
                break;
            case READ_VARIABLE:
                if(c == ','){
                    parserState = READ_PORT;
                    nextPhase = READ_PIN;
                } else if(c == ' '){

                    /* We expect spaces to already be stripped from the front of the variable name, so any spaces after mean you're done 
                    typing it. Otherwise, it will move into expect separator and error.*/
                    parserState = EXPECT_SEPARATOR;
                    nextPhase = READ_PORT;
                } else{
                    vname[vn_pointer++] = c;

                    if(vn_pointer == sizeof(vname) - 1){
                        uart_writeP(PSTR("ERR: VARIABLE NAME IS TOO LONG \r\n"));
                        return;
                    }
                }
                break;
            case DONE:
                if(c == ' '){
                    //Strip spaces
                } else{
                    uart_writeP(PSTR("ERR: UNEXPECTED CHARACTERS AT END OF EXPRESSION \r\n"));
                    return;
                }
                break;
            default:
                uart_writeP(PSTR("ERR: UNDEFINED STATE \r\n"));
                return;
        }

        input_pointer++;
    }


    if(vn_pointer == 0){
        uart_writeP(PSTR("ERR: MISSING VARIABLE PARAMETER \r\n"));
        return;
    }

    vname[vn_pointer] = '\0';

    if(pe_pointer == 0){
        uart_writeP(PSTR("ERR: MISSING PIN OR PIN EXPRESSION \r\n"));
        return;
    }

    pinExpression[pe_pointer] = '\0';

    
    int16_t pinRaw = 0;
    uint8_t pstatus = 0;

    parseExpression(pinExpression, pe_pointer, 0, &pinRaw, &pstatus);

    if(pstatus != 0){
        uart_writeP(PSTR("ERR: ERROR EVALUATING PIN EXPRESSION \r\n"));
        return;
    }

    if(pinRaw < 0 || pinRaw > 7){
        uart_writeP(PSTR("ERR: PIN VALUE MUST BE BETWEEN 0 AND 7 INCLUSIVE \r\n"));
        return;
    }

    pin = (uint8_t)pinRaw;

    volatile uint8_t * pinr;

    if(port == 'B'){
        pinr = &PINB;
    } else if(port == 'C'){
        pinr = &PINC;
    } else {
        pinr = &PIND;
    }

    value = (*pinr >> pin) & 0x01;
    uint8_t res = vr_set(vname, (int16_t)value);
    if(res == 0){
        uart_writeP(PSTR("ERR: FAILED TO SET VARIABLE \r\n"));
        return;
    }

}