/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_PROJECTTARGETSWRITER_H
#define UNITTESTBOT_PROJECTTARGETSWRITER_H

#include "TargetsWriter.h"
#include "building/BuildDatabase.h"

#include <protobuf/testgen.pb.h>

#include <vector>

class ProjectTargetsWriter : public TargetsWriter<testsgen::ProjectTargetsResponse> {
public:
    explicit ProjectTargetsWriter(testsgen::ProjectTargetsResponse *response);

    void writeResponse(const utbot::ProjectContext &projectContext,
                       const std::vector<std::shared_ptr<BuildDatabase::TargetInfo>> &targets);
};


#endif // UNITTESTBOT_PROJECTTARGETSWRITER_H
