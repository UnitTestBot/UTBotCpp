/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FunctionTESTGEN_H
#define UNITTESTBOT_FunctionTESTGEN_H

#include "LineTestGen.h"
#include "ProjectTestGen.h"

class FunctionTestGen : public LineTestGen {
public:
    FunctionTestGen(const testsgen::FunctionRequest &request,
                    ProgressWriter *progressWriter,
                    bool testMode);

    ~FunctionTestGen() override = default;

    string toString() override;

    bool needToAddPathFlag() override;
};


#endif // UNITTESTBOT_FunctionTESTGEN_H
