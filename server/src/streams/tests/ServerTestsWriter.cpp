#include "ServerTestsWriter.h"

#include "utils/FileSystemUtils.h"

#include "loguru.h"

#include <fstream>
#include <iostream>

void ServerTestsWriter::writeTestsWithProgress(tests::TestsMap &testMap,
                                               const std::string &message,
                                               const fs::path &testDirPath,
                                               std::function<void(tests::Tests &)> &&prepareTests,
                                               std::function<void()> &&prepareTotal) {
    size_t size = testMap.size();
    writeProgress(message);
    int totalTestsCounter = 0;
    for (auto it = testMap.begin(); it != testMap.end(); ++it) {
        tests::Tests &tests = it.value();
        ExecUtils::throwIfCancelled();
        prepareTests(tests);
        if (writeFileAndSendResponse(tests, testDirPath, message, (100.0 * totalTestsCounter) / size, false)) {
            ++totalTestsCounter;
        }
    }
    prepareTotal();
    writeCompleted(testMap, totalTestsCounter);
}

bool ServerTestsWriter::writeFileAndSendResponse(const tests::Tests &tests,
                                                 const fs::path &testDirPath,
                                                 const std::string &message,
                                                 double percent,
                                                 bool isCompleted) const {
    fs::path testFilePath = testDirPath / tests.relativeFileDir / tests.testFilename;
    if (!tests.code.empty()) {
        FileSystemUtils::writeToFile(testFilePath, tests.code);
    }
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

void ServerTestsWriter::writeReport(const std::string &content,
                                    const std::string &message,
                                    const fs::path &pathToStore) const
{
    if (synchronizeCode || fs::exists(pathToStore)) {
        testsgen::TestsResponse response;
        if (synchronizeCode) {
            TestsWriter::writeReport(content, message, pathToStore);
        }
        auto testSource = response.add_testsources();
        testSource->set_filepath(pathToStore);
        if (synchronizeCode) {
            testSource->set_code(content);
        }
        LOG_S(INFO) << message;
        auto progress = GrpcUtils::createProgress(message, 100, false);
        response.set_allocated_progress(progress.release());
        writeMessage(response);
    }
}