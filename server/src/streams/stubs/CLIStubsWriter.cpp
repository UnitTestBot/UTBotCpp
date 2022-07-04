#include "CLIStubsWriter.h"

#include <utils/FileSystemUtils.h>
#include "loguru.h"

void CLIStubsWriter::writeResponse(const std::vector<Stubs> &synchronizedStubs,
                                   const fs::path &testDirPath) {
    LOG_S(INFO) << "Writing stubs...";
    writeStubsFilesOnServer(synchronizedStubs, testDirPath);
    LOG_S(INFO) << "Stubs generated";
}
