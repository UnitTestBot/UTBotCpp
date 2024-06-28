#include "CLITestsWriter.h"

#include "loguru.h"

#include <utils/FileSystemUtils.h>


void CLITestsWriter::writeTestsWithProgress(tests::TestsMap &testMap,
                                            const std::string &message,
                                            const fs::path &testDirRelPath,
                                            std::function<void(tests::Tests &)> &&prepareTests,
                                            std::function<void()> &&prepareTotal) {
    std::cout << message << std::endl;
    int totalTestsCounter = 0;
    for (auto it = testMap.begin(); it != testMap.end(); ++it) {
        tests::Tests &tests = it.value();
        prepareTests(tests);
        if (writeTestFile(tests, testDirRelPath)) {
            ++totalTestsCounter;
            LOG_S(INFO) << tests.testFilename << " test file generated";
        }
    }
    prepareTotal();
    LOG_S(INFO) << "total test files generated: " << totalTestsCounter;
}

void CLITestsWriter::writeReport(const std::string &content,
                                 const std::string &message,
                                 const fs::path &pathToStore) const {
    TestsWriter::writeReport(content, message, pathToStore);
    LOG_S(INFO) << message;
}

bool CLITestsWriter::writeTestFile(const tests::Tests &tests, const fs::path &testDirRelPath) {
    fs::path testFilePath = testDirRelPath / tests.relativeFileDir / tests.testFilename;
    FileSystemUtils::writeToFile(testFilePath, tests.code);
    return !tests.code.empty();
}
