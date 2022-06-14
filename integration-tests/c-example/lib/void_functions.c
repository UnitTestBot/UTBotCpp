#include "void_functions.h"
#include <stdio.h>

void print_sign(int a) {
    if (a < 0) {
        printf("'a' is negative");
    } else if (a > 0) {
        printf("'a' is positive");
    } else {
        printf("'a' is zero");
    }
}


void print_signs_for_two_structs(struct ThisStruct thisStr1, struct ThisStruct thisStr2) {
    if (thisStr1.a * thisStr2.a > 0) {
        printf("Equal signs\n");
    } else if (thisStr1.a * thisStr2.a < 0) {
        printf("Different signs\n");
    } else {
        printf("At least one is 0\n");
    }
}

void print_value(int a) {
    if (a > 0) {
        printf("%d > 0\n", a);
        if (a > 10) {
            printf("%d > 10\n", a);
            if (a > 20) {
                printf("%d > 20\n", a);
                if (a > 30) {
                    printf("%d > 30\n", a);
                    if (a == 42) {
                        printf("%d == 42\n", a);
                    }
                }
            }
        }
    }
}
