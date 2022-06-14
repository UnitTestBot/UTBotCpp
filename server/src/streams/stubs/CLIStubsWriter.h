#ifndef UNITTESTBOT_CLISTUBSWRITER_H
#define UNITTESTBOT_CLISTUBSWRITER_H

#include "StubsWriter.h"


class CLIStubsWriter : public StubsWriter {
public:
    explicit CLIStubsWriter(): StubsWriter(nullptr) {};

    void writeResponse(const std::vector<Stubs> &synchronizedStubs, const fs::path &testDirPath) override;

};


#endif // UNITTESTBOT_CLISTUBSWRITER_H
