/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_ISTREAMWRITER_H
#define UNITTESTBOT_ISTREAMWRITER_H

#include <string>
#include <optional>

class IStreamWriter {
public:
    [[nodiscard]] virtual bool hasStream() const = 0;

    virtual ~IStreamWriter() = default;
};


#endif //UNITTESTBOT_ISTREAMWRITER_H
