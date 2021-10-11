/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "char_literals.h"

bool is_escaped_char(unsigned char x) {
    if ((int)x == 10)
        return true;
    return false;
}

bool is_printable_char(unsigned char x) {
    if ((int)x == 97)
        return true;
    return false;
}

bool is_large_char(unsigned char x) {
    if ((int)x == 217)
        return true;
    return false;
}