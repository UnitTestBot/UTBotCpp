/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TARGETSWRITER_H
#define UNITTESTBOT_TARGETSWRITER_H

#include "MessageWriter.h"
#include "building/BuildDatabase.h"

template <typename Response>
class TargetsWriter : public MessageWriter<Response> {
public:
    explicit TargetsWriter(Response *response) : MessageWriter<Response>(response) {
    }

protected:
    using MessageWriter<Response>::writer;

    void writeTargets(const std::vector<std::shared_ptr<BuildDatabase::TargetInfo>> &targets,
                      const utbot::ProjectContext &projectContext) {
        auto projectTarget = writer->add_targets();
        *projectTarget = GrpcUtils::createAutoTarget();
        for (auto const &target : targets) {
            projectTarget = writer->add_targets();
            GrpcUtils::initProjectTarget(*projectTarget, projectContext, target->getOutput());
        }
    }
};


#endif // UNITTESTBOT_TARGETSWRITER_H
