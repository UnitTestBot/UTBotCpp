/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef SIMPLE_TEST_PROJECT_ENUMS_H
#define SIMPLE_TEST_PROJECT_ENUMS_H

enum Sign {
    NEGATIVE,
    ZERO,
    POSITIVE
};

struct EnumStruct {
    enum Sign s;
};

struct EnumArrayWrapper {
    enum Sign signs[5];
};

int enumSignToInt(enum Sign s);

enum Sign intToSign(int a);

int structWithSignToInt(struct EnumStruct st);

int sumSignArray(struct EnumArrayWrapper enWrapper);

int enumSignPointerToInt(enum Sign *s);

enum Sign* intToSignPointer(int a);

int getSignValue(enum Sign s);

#endif // SIMPLE_TEST_PROJECT_ENUMS_H
