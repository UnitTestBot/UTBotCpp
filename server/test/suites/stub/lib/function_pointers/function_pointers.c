#include "function_pointers.h"

int vowel_consonant(char c, char (*vowel) (char)) {
    char s = vowel(c);
    if (s == 'a' || s == 'e' || s == 'i' || s == 'o' || s == 'u' || s == 'y') {
        return 1;
    } else {
        return -1;
    }
}
