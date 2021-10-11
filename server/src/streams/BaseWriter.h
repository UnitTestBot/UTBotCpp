/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_BASEWRITER_H
#define UNITTESTBOT_BASEWRITER_H

#include "IStreamWriter.h"
#include "WriterUtils.h"
#include "utils/GrpcUtils.h"

#include <protobuf/testgen.grpc.pb.h>

template <typename Response, typename Writer>
class BaseWriter : public virtual IStreamWriter {
protected:
    Writer *writer;

    void writeMessage(Response const &message) const {
        if (!hasStream()) {
            return;
        }
        writer->Write(message);
    }

public:
    explicit BaseWriter(Writer *writer) : writer(writer) {}

    [[nodiscard]] bool hasStream() const override {
        return writer != nullptr;
    }
};


#endif //UNITTESTBOT_BASEWRITER_H
