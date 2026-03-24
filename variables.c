#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "uart.h"
#include "parser.h"
#include "variables.h"
#include <avr/pgmspace.h>

Variable variables[12] = {0};

void vr_initall(){
    for(uint8_t i = 0; i < 12; i++){
        variables[i].is_set = 0;
        variables[i].value = 0;
    }
}

uint8_t vr_get(const char * name, int16_t * out_value){
    for(uint8_t i = 0; i < 12; i++){
        if(variables[i].is_set && strcmp(variables[i].name, name) == 0){
            *out_value = variables[i].value;
            return 1;
        }
    }
    return 0;
}

uint8_t vr_set(const char * name, int16_t value){
    for(uint8_t i = 0; i < 12; i++){
        if(variables[i].is_set && strcmp(variables[i].name, name) == 0){
            variables[i].value = value;
            return 1;
        }
    }

    //If we reach here, the variable doesn't exist yet, we should create it
    for(uint8_t i = 0; i < 12; i++){
        if(!variables[i].is_set){
            strncpy(variables[i].name, name, sizeof(variables[i].name) - 1);
            variables[i].value = value;
            variables[i].is_set = 1;
            return 1;
        }
    }

    return 0;
}

void handle_assign(const char * assignment_str)
{
    enum ParserState {EXPECT_VARIABLE_NAME, EXPECT_EQUAL, READING_VARIABLE_NAME, READING_EXPRESSION} state = EXPECT_VARIABLE_NAME;
    char vname[9] = {0};
    uint8_t vname_pointer = 0;
    uint8_t input_pointer = 0;

    uint8_t len = strlen(assignment_str);

    uint8_t status = 0;
    int16_t value = 0;

    while(1){

        if(input_pointer >= len){
            //We've reached the end of the input without finishing parsing, this is an error
            uart_writeP(PSTR("ERR: INVALID ASSIGNMENT"));
            return;
        }

        char c = assignment_str[input_pointer];

        if(state == EXPECT_VARIABLE_NAME){

            if(c == ' ')
            {
                //Ignore blank space when looking for a variable name
                input_pointer++;
                continue;
            } else if(isdigit((unsigned char)c)){
                //This is an error, variables have to start with an alphabetical character
                uart_writeP(PSTR("ERR: INVALID VARIABLE NAME"));
                return;
            } else if(isalpha((unsigned char)c)){
                state = READING_VARIABLE_NAME;
            } else{
                uart_writeP(PSTR("ERR: UNKNOWN ASSIGNMENT TOKEN"));
                return;
            }

        } else if(state == READING_VARIABLE_NAME){

            if(c == ' ' || c == '='){
                //Variable names can't contain spaces or equal, so its the end of a variable (or an error)
                state = EXPECT_EQUAL;
                vname[vname_pointer] = '\0';
            } else if(isalnum((unsigned char)c)){

                if(vname_pointer < sizeof(vname) - 1){
                    vname[vname_pointer++] = c;
                    input_pointer++;
                } else{
                    //Variable name is too long, we should throw an error
                    uart_writeP(PSTR("ERR: VARIABLE NAME TOO LONG"));
                    return;
                }
            } else{
                uart_writeP(PSTR("ERR: INVALID CHARACTER IN VARIABLE NAME"));
                return;
            }

        } else if(state == EXPECT_EQUAL) {

            if(c == ' '){
                //We strip all spaces up till the equal sign, no big deal
                input_pointer++;
            } else if(c == '='){
                //We found our equal sign, we can move on to reading the expression
                state = READING_EXPRESSION;
                input_pointer++;
            } else{
                //This is an error, we should have found an equal sign after the variable name
                uart_writeP(PSTR("ERR: EXPECTED EQUAL SIGN"));
                return;
            }

        } else if(state == READING_EXPRESSION){
            input_pointer = parseExpression(assignment_str, len, input_pointer, &value, &status);

            if(status != 0){
                uart_writeP(PSTR("ERR: INVALID EXPRESSION"));
                return;
            }

            while(input_pointer < len){
                if(assignment_str[input_pointer] == ' '){
                    input_pointer++;
                }else{
                    //This is an error, we should only have found spaces after the expression
                    uart_writeP(PSTR("ERR: UNEXPECTED TOKEN AFTER EXPRESSION"));
                    return;
                }
            }
            break;
        } else{
            //This should never happen, but just in case
            uart_writeP(PSTR("ERR: INTERNAL PARSER ERROR"));
            return;
        }


    }

    //If you reach here, the assignment was parsed and the variable can be assigned
    uint8_t result = vr_set(vname, value);
    if(!result){
        uart_writeP(PSTR("ERR: FAILED TO SET VARIABLE\r\n"));
    }

    return;

}