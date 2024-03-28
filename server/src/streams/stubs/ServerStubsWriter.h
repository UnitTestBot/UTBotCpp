#ifndef UNITTESTBOT_VSCODESTUBSWRITER_H
#define UNITTESTBOT_VSCODESTUBSWRITER_H

#include "StubsWriter.h"

class ServerStubsWriter : public StubsWriter {
public:
    explicit ServerStubsWriter(grpc::ServerWriter<testsgen::StubsResponse> *writer, bool synchronizeCode)
        : StubsWriter(writer), synchronizeCode(synchronizeCode) {
    }

    void writeResponse(const std::vector<Stubs> &synchronizedStubs,
                       const fs::path &testDirRelPath) override;
private:
    bool synchronizeCode;
};


#endif // UNITTESTBOT_VSCODESTUBSWRITER_H
