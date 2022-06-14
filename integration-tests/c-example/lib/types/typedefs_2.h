#ifndef SIMPLE_TEST_PROJECT_CHECK_TYPEDEF_2_H
#define SIMPLE_TEST_PROJECT_CHECK_TYPEDEF_2_H

typedef enum __Sign {
    NEG1,
    ZER1,
    POS1
} Sign1;

typedef enum {
    NEG2,
    ZER2,
    POS2
} Sign2;

int enumSign1ToInt(Sign1 s);

Sign1 intToSign1(int a);

int enumSign2ToInt(Sign2 s);

Sign2 intToSign2(int a);

#endif
