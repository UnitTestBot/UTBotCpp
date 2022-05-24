/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "FileTargetsWriter.h"

void FileTargetsWriter::writeResponse(
    const std::vector<std::shared_ptr<BuildDatabase::TargetInfo>> &targets,
    const utbot::ProjectContext &projectContext) {
    if (!hasStream()) {
        return;
    }
    writeTargets(targets, projectContext);
}
