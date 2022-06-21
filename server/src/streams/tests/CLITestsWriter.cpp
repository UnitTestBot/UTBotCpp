/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "CLITestsWriter.h"

#include "loguru.h"

#include <utils/FileSystemUtils.h>


void CLITestsWriter::writeTestsWithProgress(tests::TestsMap &testMap,
                                            const string &message,
                                            const fs::path &testDirPath,
                                            std::function<void(tests::Tests &)> &&functor,
                                            const utbot::ProjectContext &projectContext,
                                            const utbot::SettingsContext &settingsContext) {
    size_t size = testMap.size();
    std::cout << message << std::endl;
    int totalTestsCounter = 0;
    for (auto it = testMap.begin(); it != testMap.end(); it++) {
        tests::Tests& tests = it.value();
        functor(tests);
        if (writeTestFile(tests, testDirPath)) {
            totalTestsCounter += 1;
            auto generatedMessage = StringUtils::stringFormat("%s test file generated", tests.testFilename);
            LOG_S(INFO) << generatedMessage;
        }
    }
    std::string finalMessage;
    if (totalTestsCounter == 1) {
        finalMessage = StringUtils::stringFormat("%d test file generated.", totalTestsCounter);
    } else {
        finalMessage = StringUtils::stringFormat("%d test files generated.", totalTestsCounter);
    }
    LOG_S(INFO) << finalMessage;

}
bool CLITestsWriter::writeTestFile(const tests::Tests &tests, const fs::path &testDirPath) {
    fs::path testFilePath = testDirPath / tests.relativeFileDir / tests.testFilename;
    FileSystemUtils::writeToFile(testFilePath, tests.code);
    return !tests.code.empty();
}
