#include "CLITestsWriter.h"

#include "loguru.h"

#include <utils/FileSystemUtils.h>


void CLITestsWriter::writeTestsWithProgress(tests::TestsMap &testMap,
                                            const std::string &message,
                                            const fs::path &testDirPath,
                                            std::function<void(tests::Tests &)> &&prepareTests,
                                            std::function<void()> &&prepareTotal) {
    std::cout << message << std::endl;
    int totalTestsCounter = 0;
    for (auto it = testMap.begin(); it != testMap.end(); ++it) {
        tests::Tests& tests = it.value();
        prepareTests(tests);
        if (writeTestFile(tests, testDirPath)) {
            ++totalTestsCounter;
            LOG_S(INFO) << tests.testFilename << " test file generated";
        }
    }
    prepareTotal();
    LOG_S(INFO) << "total test files generated: " << totalTestsCounter;
}

bool CLITestsWriter::writeTestFile(const tests::Tests &tests, const fs::path &testDirPath) {
    fs::path testFilePath = testDirPath / tests.relativeFileDir / tests.testFilename;
    FileSystemUtils::writeToFile(testFilePath, tests.code);
    return !tests.code.empty();
}
