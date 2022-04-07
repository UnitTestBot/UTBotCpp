/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_LineTESTGEN_H
#define UNITTESTBOT_LineTESTGEN_H

#include "ProjectTestGen.h"

class LineTestGen : public ProjectTestGen {
public:
    LineTestGen(const testsgen::LineRequest &request,
                ProgressWriter *progressWriter,
                bool testMode,
                bool forHeader = false);
    
    ~LineTestGen() override = default;

    const fs::path &getSourcePath() const;

    string toString() override;

    virtual bool needToAddPathFlag();

    fs::path filePath;
    uint32_t line;
};


#endif // UNITTESTBOT_LineTESTGEN_H
