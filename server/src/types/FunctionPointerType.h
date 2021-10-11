/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FUNCTION_POINTERTYPE_H
#define UNITTESTBOT_FUNCTION_POINTERTYPE_H

#include "AbstractType.h"

class FunctionPointerType: public AbstractType {
public:
    Kind getKind() override {
        return FUNCTION_POINTER;
    }
};
#endif