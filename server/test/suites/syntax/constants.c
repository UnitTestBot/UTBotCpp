#include "constants.h"

bool is_signed_char_min(signed char value) {
    if (value == SCHAR_MIN) {
        return true;
    }
    return false;
}

bool is_signed_char_max(signed char value) {
    if (value == SCHAR_MAX) {
        return true;
    }
    return false;
}

bool is_unsigned_char_max(unsigned char value) {
    if (value == UCHAR_MAX) {
        return true;
    }
    return false;
}

bool is_short_min(short value) {
    if (value == SHRT_MIN) {
        return true;
    }
    return false;
}

bool is_short_max(short value) {
    if (value == SHRT_MAX) {
        return true;
    }
    return false;
}

bool is_unsigned_short_max(unsigned short value) {
    if (value == USHRT_MAX) {
        return true;
    }
    return false;
}

bool is_unsigned_int_max(unsigned int value) {
    if (value == UINT_MAX) {
        return true;
    }
    return false;
}

bool is_long_long_max(long long int value) {
    if (value == LLONG_MAX) {
        return true;
    }
    return false;
}

bool is_long_long_min(long long int value) {
    if (value == LLONG_MIN) {
        return true;
    }
    return false;
}

bool is_unsigned_long_long_max(unsigned long long int value) {
    if (value == ULLONG_MAX) {
        return true;
    }
    return false;
}
