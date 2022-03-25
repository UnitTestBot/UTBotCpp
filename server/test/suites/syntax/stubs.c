/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "stubs.h"

int find_maximum(int x, int y, int (*compare) (int, int)) {
    int t = compare(x, y);
    if (t) {
        return x;
    } else {
        return y;
    }
}

int vowel_consonant(char c, char (*vowel) (char)) {
    char s = vowel(c);
    if (s == 'a' || s == 'e' || s == 'i' || s == 'o' || s == 'u' || s == 'y') {
        return 1;
    } else {
        return -1;
    }
}
