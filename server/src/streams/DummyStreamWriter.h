/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_DUMMYSTREAMWRITER_H
#define UNITTESTBOT_DUMMYSTREAMWRITER_H

#include "ProgressWriter.h"

class DummyStreamWriter : ProgressWriter {
public:
    [[nodiscard]] bool hasStream() const override;

    void writeProgress(const std::optional<std::string> &message,
                       double percent,
                       bool completed) const override;

    static ProgressWriter const* getInstance();
};



#endif //UNITTESTBOT_DUMMYSTREAMWRITER_H
