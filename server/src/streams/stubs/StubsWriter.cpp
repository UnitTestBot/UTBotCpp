#include "StubsWriter.h"

#include "Paths.h"
#include "stubs/StubGen.h"
#include "utils/FileSystemUtils.h"

#include "loguru.h"

StubsWriter::StubsWriter(grpc::ServerWriter<testsgen::StubsResponse> *writer) : ServerWriter(writer) {
}

void StubsWriter::writeStubsFilesOnServer(const std::vector<Stubs> &stubs, const fs::path &testDirPath) {
    for (const auto &stub : stubs) {
        FileSystemUtils::writeToFile(stub.filePath, stub.code);
    }
}
