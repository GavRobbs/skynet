#include "parser.h"
#include <ctype.h>
#include "variables.h"

uint8_t findNextTokenStart(const char *scan_str, uint8_t explen, uint8_t pointer_index, uint8_t *status)
{
    // status
    // 0 - no error
    // 1 - end of input - technically a normal termination
    // 2 - no valid token found
    uint8_t new_index = pointer_index;
    char c = ' ';

    while (1)
    {
        if (new_index >= explen)
        {
            *status = 1;
            return explen;
        }

        c = scan_str[new_index];

        if (c == ' ')
        {
            // Skip spaces
        }
        else
        {
            if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || isalnum((unsigned char)c))
            {
                *status = 0;
                return new_index;
            }
            else
            {
                *status = 2;
                return 0;
            }
        }

        new_index++;
    }
}

uint8_t parseFactor(const char *factorStr, uint8_t explen, uint8_t pointer_index, int16_t *result, uint8_t *status)
{
    /* F -> ID | INTEGER | ( E ) | -F  */
    // status
    // 0 - no error
    // 1 - end of input
    // 2 - no valid token found
    // 3 - variable name too long
    // 4 - invalid character in factor
    // 5 - variable not found

    uint8_t new_pointer = findNextTokenStart(factorStr, explen, pointer_index, status);

    if (*status == 1)
    {
        *result = 0;
        return new_pointer;
    }
    else if (*status != 0)
    {
        *status = 4;
        *result = 0;
        return new_pointer;
    }

    char c = factorStr[new_pointer];
    int16_t temp = 0;
    char vname[9] = {0};
    uint8_t vn_pointer = 0;

    if (c == '(')
    {
        // We've started a parenthesized expression here
        new_pointer = parseExpression(factorStr, explen, new_pointer + 1, result, status);
        if (*status != 0)
        {
            *result = 0;
            return new_pointer;
        }

        new_pointer = findNextTokenStart(factorStr, explen, new_pointer, status);

        if (*status == 1)
        {
            // Missing closing parenthesis → error
            *status = 4;
            *result = 0;
            return new_pointer;
        }
        else if (*status != 0)
        {
            *status = 4;
            *result = 0;
            return new_pointer;
        }

        c = factorStr[new_pointer];

        if (c == ')')
        {
            *status = 0;
            return new_pointer + 1;
        }
        else
        {
            *status = 4;
            *result = 0;
            return new_pointer;
        }
    }
    else if (isdigit((unsigned char)c))
    {

        do
        {

            temp = (int16_t)temp * 10 + (c - '0');
            new_pointer++;

            if (new_pointer < explen)
            {
                c = factorStr[new_pointer];
                *status = 0;
            }
            else
            {
                *status = 0;
                break;
            }

        } while (isdigit((unsigned char)c));

        /* The loop exits when all digits have been consumed, whether by running into something else or EOL. The only valid other things are spaces, operators and closing parenthesis - if it detects an alphabetic character, you may have a case like 123abc, which is an error/ */

        if (new_pointer < explen)
        {

            c = factorStr[new_pointer];

            if (isalpha((unsigned char)c))
            {
                *status = 4;
                *result = 0;
            }
            else if (c == ' ' || c == ')' || c == '+' || c == '-' || c == '*' || c == '/')
            {
                *status = 0;
                *result = temp;
            }
            else
            {
                *status = 4;
                *result = 0;
            }
        }
        else
        {
            *result = temp;
        }
    }
    else if (isalpha((unsigned char)c))
    {

        do
        {

            if ((vn_pointer + 1) < 9)
            {
                // Leave a space for our null character
                // Variable names should be 8 characters long at most
                // No automatic truncation
                vname[vn_pointer] = c;
                vn_pointer++;
                new_pointer++;
                *status = 0;
            }
            else
            {
                *status = 3;
                break;
            }

            if (new_pointer < explen)
            {
                c = factorStr[new_pointer];
                *status = 0;
            }
            else
            {
                *status = 0;
                break;
            }

        } while (isalnum((unsigned char)c));

        /* The loop exits when all letters in the variable have been consumed, whether by running into something else or EOL. The only valid other things are spaces, operators and closing parenthesis. */

        if (new_pointer < explen)
        {

            c = factorStr[new_pointer];

            if (c == ' ' || c == ')' || c == '+' || c == '-' || c == '*' || c == '/')
            {
                *status = 0;
                vname[vn_pointer] = '\0';

                // Placeholder, should fetch variable name
                uint8_t outcome = vr_get(vname, result);
                if (outcome == 0)
                {
                    *status = 5;
                    *result = 0;
                }
            }
            else
            {
                *status = 4;
                *result = 0;
            }
        }
        else
        {
            vname[vn_pointer] = '\0';

            uint8_t outcome = vr_get(vname, result);
            if (outcome == 0)
            {
                *status = 5;
                *result = 0;
            }
        }
    }
    else if (c == '-')
    {

        new_pointer += 1;
        new_pointer = parseFactor(factorStr, explen, new_pointer, result, status);

        if (*status == 0)
        {
            *status = 0;
            *result = -1 * (*result);
        }
        else
        {
            *result = 0;
        }
    }
    else
    {

        // Invalid character

        *result = 0;
        *status = 4;
    }

    return new_pointer;
}

uint8_t parseTerm(const char *termStr, uint8_t explen, uint8_t pointer_index, int16_t *result, uint8_t *status)
{
    // status
    // 0 - no error
    // 1 - left factor parsing error
    // 2 - right factor parsing error
    // 3 - unexpected token after factor in term
    uint8_t new_pointer = pointer_index;

    if (*status != 0)
    {
        *result = 0;
        return new_pointer;
    }

    int16_t lfactor = 0;
    new_pointer = parseFactor(termStr, explen, new_pointer, &lfactor, status);

    if (*status != 0)
    {
        *result = 0;
        *status = 1;
        return new_pointer;
    }

    do
    {

        // Find the operator, if there is one
        new_pointer = findNextTokenStart(termStr, explen, new_pointer, status);

        if (*status == 0 && new_pointer < explen)
        {
            // We found an operator, we can continue
        }
        else
        {
            if (*status == 1)
            {
                // End of input is a valid terminator for a term, we can just exit
                *status = 0;
            }
            else
            {
                // Some other error, we should report it
                *status = 3;
            }
            break;
        }

        char c = termStr[new_pointer];

        if (c == '*')
        {
            // Find the right factor
            int16_t rfactor = 0;
            new_pointer = parseFactor(termStr, explen, new_pointer + 1, &rfactor, status);

            if (*status != 0)
            {
                *result = 0;
                *status = 2;
                return new_pointer;
            }

            lfactor = lfactor * rfactor;
        }
        else if (c == '/')
        {
            // Find the right factor
            int16_t rfactor = 0;
            new_pointer = parseFactor(termStr, explen, new_pointer + 1, &rfactor, status);

            if (*status != 0)
            {
                *result = 0;
                *status = 2;
                return new_pointer;
            }

            lfactor = lfactor / rfactor;
        }
        else
        {
            if (c == ')' || c == '+' || c == '-')
            {
                // No more operators, we're done with this term
                *status = 0;
            }
            else
            {
                *status = 3;
            }
            break;
        }

    } while (*status == 0);

    if (*status == 0)
    {
        *result = lfactor;
    }
    else
    {
        *result = 0;
    }

    return new_pointer;
}

uint8_t parseExpression(const char *expressionStr, uint8_t explen, uint8_t pointer_index, int16_t *result, uint8_t *status)
{
    // status
    // 0 - no error
    // 1 - left term parsing error
    // 2 - right term parsing error
    // 3 - unexpected token after term in expression
    uint8_t new_pointer = pointer_index;

    int16_t lterm = 0;
    new_pointer = parseTerm(expressionStr, explen, new_pointer, &lterm, status);

    if (*status != 0)
    {
        *result = 0;
        *status = 1;
        return new_pointer;
    }

    do
    {

        // Find the operator, if there is one
        new_pointer = findNextTokenStart(expressionStr, explen, new_pointer, status);

        if (*status == 0 && new_pointer < explen)
        {
            // We found an operator, we can continue
        }
        else if (*status == 1)
        {
            *status = 0;
            break;
        }
        else
        {
            // Some other error, we should report it
            *status = 3;
            break;
        }

        char c = expressionStr[new_pointer];

        if (c == '+')
        {
            // Find the right term
            int16_t rterm = 0;
            new_pointer = parseTerm(expressionStr, explen, new_pointer + 1, &rterm, status);

            if (*status != 0)
            {
                *result = 0;
                *status = 2;
                return new_pointer;
            }

            lterm = lterm + rterm;
        }
        else if (c == '-')
        {
            // Find the right term
            int16_t rterm = 0;
            new_pointer = parseTerm(expressionStr, explen, new_pointer + 1, &rterm, status);

            if (*status != 0)
            {
                *result = 0;
                *status = 2;
                return new_pointer;
            }

            lterm = lterm - rterm;
        }
        else
        {
            if (c == ')')
            {
                // No more operators, we're done with this expression
                *status = 0;
            }
            else
            {
                *status = 3;
            }
            break;
        }

    } while (*status == 0);

    if (*status == 0)
    {
        *result = lterm;
    }
    else
    {
        *result = 0;
    }

    return new_pointer;
}