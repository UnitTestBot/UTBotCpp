/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_AssertionTESTGEN_H
#define UNITTESTBOT_AssertionTESTGEN_H

#include "LineTestGen.h"
#include "ProjectTestGen.h"

class AssertionTestGen final : public LineTestGen {
public:
    explicit AssertionTestGen(const testsgen::AssertionRequest &request,
                              ProgressWriter *progressWriter,
                              bool testMode);

    ~AssertionTestGen() override = default;

    std::string toString() override;
};


#endif // UNITTESTBOT_AssertionTESTGEN_H
