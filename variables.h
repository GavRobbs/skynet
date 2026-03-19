#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdint.h>

typedef struct{

    char name[9];
    int16_t value;
    uint8_t is_set;

} Variable;

void handle_assign(const char * assignment_str);
void vr_initall();
uint8_t vr_get(const char * name, int16_t * out_value);
uint8_t vr_set(const char * name, int16_t value);

extern Variable variables[12];

#endif