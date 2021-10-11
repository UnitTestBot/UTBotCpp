/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_ARRAYTYPE_H
#define UNITTESTBOT_ARRAYTYPE_H

#include "AbstractType.h"

#include <cstdint>

class ArrayType : public AbstractType {
public:
    explicit ArrayType(unsigned long int size, bool complete) : size(size), complete(complete) {}

    unsigned long int getSize() const override {
        return size;
    }

    Kind getKind() override {
        return ARRAY;
    }

    bool isComplete() const {
        return complete;
    }
private:
    unsigned long int size;
    bool complete;
};

#endif