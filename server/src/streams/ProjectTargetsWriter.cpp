/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ProjectTargetsWriter.h"

ProjectTargetsWriter::ProjectTargetsWriter(testsgen::ProjectTargetsResponse *response)
    : TargetsWriter(response) {
}

void ProjectTargetsWriter::writeResponse(
    const utbot::ProjectContext &projectContext,
    const vector<std::shared_ptr<BuildDatabase::TargetInfo>> &targets) {
    if (!hasStream()) {
        return;
    }
    writeTargets(targets, projectContext);
    auto utbotAutoTarget = std::make_unique<testsgen::ProjectTarget>(GrpcUtils::createAutoTarget());
    writer->set_allocated_prioritytarget(utbotAutoTarget.release());
}
