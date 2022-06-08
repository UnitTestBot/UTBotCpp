#ifndef SIMPLE_TEST_PROJECT_LITERALS_H
#define SIMPLE_TEST_PROJECT_LITERALS_H

#include "stdbool.h"
#include "limits.h"

bool is_signed_char_min(signed char value);

bool is_signed_char_max(signed char value);

bool is_unsigned_char_max(unsigned char value);

bool is_short_min(short value);

bool is_short_max(short value);

bool is_unsigned_short_max(unsigned short value);

bool is_unsigned_int_max(unsigned int value);

bool is_long_long_max(long long value);

bool is_long_long_min(long long value);

bool is_unsigned_long_long_max(unsigned long long int value);

#endif //SIMPLE_TEST_PROJECT_LITERALS_H
