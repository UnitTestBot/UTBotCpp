/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SERVERUTIL_H
#define UNITTESTBOT_SERVERUTIL_H

#include "ThreadSafeContainers.h"

#include <grpcpp/impl/codegen/server_context.h>

namespace ServerUtils {
    using std::string;

    void setThreadOptions(grpc::ServerContext *context, bool testMode);

    void registerClient(concurrent_set<string> &clients, string client);

    void loadClientsData(concurrent_set<string> &result);
}

#endif //UNITTESTBOT_SERVERUTIL_H
