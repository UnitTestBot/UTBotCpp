/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ServerTestsWriter.h"

#include <utils/FileSystemUtils.h>


void ServerTestsWriter::writeTestsWithProgress(tests::TestsMap &testMap,
                                               std::string const &message,
                                               const fs::path &testDirPath,
                                               std::function<void(tests::Tests &)> &&functor) {

    size_t size = testMap.size();
    writeProgress(message);
    int totalTestsCounter = 0;
    for (auto it = testMap.begin(); it != testMap.end(); it++) {
        tests::Tests &tests = it.value();
        ExecUtils::throwIfCancelled();
        functor(tests);
        if (writeFileAndSendResponse(tests, testDirPath, message, 100.0 / size, false)) {
            totalTestsCounter += 1;
        }
    }
    writeCompleted(testMap, totalTestsCounter);
}

void ServerTestsWriter::writeStubs(const vector<Stubs> &synchronizedStubs) {
    testsgen::TestsResponse response;
    auto stubsResponse = std::make_unique<testsgen::StubsResponse>();
    for (auto const &synchronizedStub : synchronizedStubs) {
        auto sData = stubsResponse->add_stubsources();
        sData->set_filepath(synchronizedStub.filePath);
        if (synchronizeCode) {
            sData->set_code(synchronizedStub.code);
        }
    }
    response.set_allocated_stubs(stubsResponse.release());
}

bool ServerTestsWriter::writeFileAndSendResponse(const tests::Tests &tests,
                                                 const fs::path &testDirPath,
                                                 const string &message,
                                                 double percent,
                                                 bool isCompleted) const {
    fs::path testFilePath = testDirPath / tests.relativeFileDir / tests.testFilename;
    FileSystemUtils::writeToFile(testFilePath, tests.code);
    if (!hasStream()) {
        return false;
    }
    testsgen::TestsResponse response;
    LOG_S(DEBUG) << "Creating final response.";
    bool isAnyTestsGenerated = false;
    if (!tests.code.empty()) {
        isAnyTestsGenerated = true;
        auto testSource = response.add_testsources();
        testSource->set_filepath(tests.testSourceFilePath);
        if (synchronizeCode) {
            testSource->set_code(tests.code);
            testSource->set_errormethodsnumber(tests.errorMethodsNumber);
            testSource->set_regressionmethodsnumber(tests.regressionMethodsNumber);
        }

        auto testHeader = response.add_testsources();
        testHeader->set_filepath(tests.testHeaderFilePath);
        if (synchronizeCode) {
            testHeader->set_code(tests.headerCode);
        }
    }
    LOG_S(INFO) << message;
    auto progress = GrpcUtils::createProgress(message, percent, isCompleted);
    response.set_allocated_progress(progress.release());
    writeMessage(response);
    return isAnyTestsGenerated;
}
