#ifndef PRINT_H
#define PRINT_H

#include <stdint.h>
#include <stdlib.h>

void handle_print(const char * values);
uint8_t _print_resolve_value(char * value, char * out_string, uint8_t current_pointer);

#endif
