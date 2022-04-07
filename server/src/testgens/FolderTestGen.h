/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FolderTESTGEN_H
#define UNITTESTBOT_FolderTESTGEN_H

#include "ProjectTestGen.h"


class FolderTestGen final : public ProjectTestGen {
public:
    string folderPath;

    FolderTestGen(const testsgen::FolderRequest &request,
                  ProgressWriter *progressWriter,
                  bool testMode);

    ~FolderTestGen() override = default;

    string toString() override;
};


#endif // UNITTESTBOT_FolderTESTGEN_H
