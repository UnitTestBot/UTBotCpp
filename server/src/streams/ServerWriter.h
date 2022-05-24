/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_SERVERWRITER_H
#define UNITTESTBOT_SERVERWRITER_H

#include "BaseWriter.h"
#include "ProgressWriter.h"

namespace utbot {
    template <typename Response>
    class ServerWriter : public BaseWriter<Response, grpc::ServerWriter<Response>>,
                         public ProgressWriter {
    public:
        explicit ServerWriter(grpc::ServerWriter<Response> *writer)
            : BaseWriter<Response, grpc::ServerWriter<Response>>(writer) {
        }

        void writeProgress(const std::optional<std::string> &message,
                           double percent = 0.0,
                           bool completed = false) const override {
            if (!this->hasStream()) {
                return;
            }
            Response response;
            auto progress = GrpcUtils::createProgress(message, percent, completed);
            response.set_allocated_progress(progress.release());
            this->writeMessage(response);
        }
    };
}


#endif // UNITTESTBOT_SERVERWRITER_H
