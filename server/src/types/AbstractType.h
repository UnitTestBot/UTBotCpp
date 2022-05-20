/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_ABSTRACTTYPE_H
#define UNITTESTBOT_ABSTRACTTYPE_H

class AbstractType {
public:
    enum Kind {
        OBJECT_POINTER,
        FUNCTION_POINTER,
        ARRAY,
        SIMPLE
    };

    [[nodiscard]] virtual unsigned long int getSize() const {
        return 0;
    }

    [[nodiscard]] virtual Kind getKind() const = 0;
};

#endif
