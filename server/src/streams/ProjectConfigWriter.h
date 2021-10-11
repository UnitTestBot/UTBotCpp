/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_PROJECTCONFIGWRITER_H
#define UNITTESTBOT_PROJECTCONFIGWRITER_H

#include "ServerWriter.h"

#include <grpcpp/grpcpp.h>
#include <protobuf/testgen.grpc.pb.h>

class ProjectConfigWriter : public utbot::ServerWriter<testsgen::ProjectConfigResponse> {
public:
    explicit ProjectConfigWriter(grpc::ServerWriter<testsgen::ProjectConfigResponse> *writer)
        : ServerWriter(writer) {
    }

    void writeResponse(testsgen::ProjectConfigStatus status,
                       std::optional<std::string> const &message = std::nullopt) const;
};


#endif // UNITTESTBOT_PROJECTCONFIGWRITER_H
