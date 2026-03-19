#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdlib.h>

uint8_t findNextTokenStart(const char *scan_str, uint8_t explen, uint8_t pointer_index, uint8_t *status);
uint8_t parseFactor(const char *factorStr, uint8_t explen, uint8_t pointer_index, int16_t *result, uint8_t *status);
uint8_t parseTerm(const char *termStr, uint8_t explen, uint8_t pointer_index, int16_t *result, uint8_t *status);
uint8_t parseExpression(const char *expressionStr, uint8_t explen, uint8_t pointer_index, int16_t *result, uint8_t *status);

#endif
