/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_REQUESTENVIRONMENT_H
#define UNITTESTBOT_REQUESTENVIRONMENT_H

#include <grpcpp/grpcpp.h>

namespace RequestEnvironment {
    extern thread_local std::optional<std::string> clientId;
    extern thread_local grpc::ServerContext *serverContext;

    const std::string &getClientId();
    const grpc::ServerContext *getServerContext();
    void setClientId(std::string requestClientId);
    void setServerContext(grpc::ServerContext *requestServerContext);
    bool isCancelled();
};


#endif // UNITTESTBOT_REQUESTENVIRONMENT_H
