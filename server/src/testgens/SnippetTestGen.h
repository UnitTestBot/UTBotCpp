/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SNIPPETTESTGEN_H
#define UNITTESTBOT_SNIPPETTESTGEN_H

#include "BaseTestGen.h"

class SnippetTestGen final : public BaseTestGen {
public:
    SnippetTestGen(const testsgen::SnippetRequest &request,
                   ProgressWriter *progressWriter,
                   bool testMode);

    ~SnippetTestGen() override = default;

    string toString() override;

    fs::path filePath;

private:
    void setTargetForSource(const fs::path &sourcePath) override;
};


#endif // UNITTESTBOT_SNIPPETTESTGEN_H
