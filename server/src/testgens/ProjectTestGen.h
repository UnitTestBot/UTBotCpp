/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_PROJECTTESTGEN_H
#define UNITTESTBOT_PROJECTTESTGEN_H

#include "BaseTestGen.h"
#include "ProjectTarget.h"

#include <optional>

class ProjectTestGen : public BaseTestGen {
public:
    ProjectTestGen(const testsgen::ProjectRequest &request,
                   ProgressWriter *progressWriter,
                   bool testMode);

    ~ProjectTestGen() override = default;

    string toString() override;

    const testsgen::ProjectRequest *getRequest() const;
    
    void setTargetForSource(fs::path const &sourcePath) override;

private:
    testsgen::ProjectRequest const *const request;
};


#endif // UNITTESTBOT_PROJECTTESTGEN_H
