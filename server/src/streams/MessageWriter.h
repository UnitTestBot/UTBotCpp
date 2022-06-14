#ifndef UNITTESTBOT_MESSAGEWRITER_H
#define UNITTESTBOT_MESSAGEWRITER_H

#include "BaseWriter.h"
#include "exceptions/UnImplementedException.h"

template <typename Response> class MessageWriter : public BaseWriter<Response, Response> {
public:
    explicit MessageWriter(Response *response) : BaseWriter<Response, Response>(response) {
    }

protected:
    using BaseWriter<Response, Response>::writer;
};


#endif // UNITTESTBOT_MESSAGEWRITER_H
