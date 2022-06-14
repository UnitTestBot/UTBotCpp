#ifndef UNITTESTBOT_CHAR_LITERALS_H
#define UNITTESTBOT_CHAR_LITERALS_H
#include <stdbool.h>

bool is_escape_char(unsigned char x);

bool is_printable_char(unsigned char x);

bool is_large_char(unsigned char x);
#endif //UNITTESTBOT_CHAR_LITERALS_H
