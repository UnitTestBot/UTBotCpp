/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_OBJECTPOINTERTYPE_H
#define UNITTESTBOT_OBJECTPOINTERTYPE_H

#include "AbstractType.h"

class ObjectPointerType : public AbstractType {
    bool constQualified;

public:
    explicit ObjectPointerType(bool constQualified) : constQualified(constQualified) {
    }

    Kind getKind() override {
        return OBJECT_POINTER;
    }

    bool isConstQualified() const {
        return constQualified;
    }
};
#endif