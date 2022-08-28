#include "ServerStubsWriter.h"

#include "loguru.h"

void ServerStubsWriter::writeResponse(const std::vector<Stubs> &synchronizedStubs,
                                      const fs::path &testDirPath,
                                      const std::string &message) {
    writeStubsFilesOnServer(synchronizedStubs, testDirPath);
    if (!hasStream()) {
        return;
    }
    LOG_S(DEBUG) << "Writing stubs with progress.";
    for (size_t i = 0; i < synchronizedStubs.size(); i++) {
        testsgen::StubsResponse response;
        auto sData = response.add_stubsources();
        sData->set_filepath(synchronizedStubs[i].filePath);
        if (synchronizeCode) {
            sData->set_code(synchronizedStubs[i].code);
        }
        auto progress = GrpcUtils::createProgress(message, (100.0 * i) / synchronizedStubs.size(), true);
        response.set_allocated_progress(progress.release());
        writeMessage(response);
    }
}
