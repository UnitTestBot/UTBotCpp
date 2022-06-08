#ifndef UNITTESTBOT_STUBSWRITER_H
#define UNITTESTBOT_STUBSWRITER_H

#include "streams/ServerWriter.h"
#include "stubs/Stubs.h"

#include <protobuf/testgen.pb.h>

#include "utils/path/FileSystemPath.h"

class StubsWriter : public utbot::ServerWriter<testsgen::StubsResponse> {
public:
    explicit StubsWriter(grpc::ServerWriter<testsgen::StubsResponse> *writer);

    virtual void writeResponse(const std::vector<Stubs> &synchronizedStubs, const fs::path &testDirPath) = 0;

    static void writeStubsFilesOnServer(const std::vector<Stubs> &stubs, const fs::path &testDirPath);

};


#endif // UNITTESTBOT_STUBSWRITER_H
