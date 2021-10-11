/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TESTUTILS_H
#define UNITTESTBOT_TESTUTILS_H

#include "gtest/gtest.h"

#include "ProjectTarget.h"
#include "Server.h"
#include "Tests.h"
#include "coverage/Coverage.h"
#include "coverage/UnitTest.h"
#include "utils/CollectionUtils.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

using Params = const std::vector<shared_ptr<tests::AbstractValueView>> &;
using ReturnValue = const std::shared_ptr<tests::AbstractValueView> &;
using TestCasePredicate = std::function<bool(tests::Tests::MethodTestCase)>;
using CoverageLines = CollectionUtils::MapFileTo<std::set<int>>;

namespace testUtils {
    using std::function;
    using std::string;
    using std::vector;

    enum BuildCommandsTool {
        BEAR_BUILD_COMMANDS_TOOL,
        CMAKE_BUILD_COMMANDS_TOOL,
        MAKE_BUILD_COMMANDS_TOOL
    };

    void checkTestCasePredicates(const vector<tests::Tests::MethodTestCase> &testCases,
                                 const vector<TestCasePredicate> &predicates,
                                 const string& functionName = "");

    void checkCoverage(const Coverage::CoverageMap &coverageMap,
                       const CoverageLines &expectedLinesCovered,
                       const CoverageLines &expectedLinesUncovered,
                       const CoverageLines &expectedLinesNone);

    void checkStatuses(const Coverage::TestStatusMap &testStatusMap, const vector<UnitTest> &tests);

    int getNumberOfTests(const tests::TestsMap &tests);

    void checkMinNumberOfTests(const tests::TestsMap &tests, int minNumber);

    void checkMinNumberOfTests(const std::vector<tests::Tests::MethodTestCase> &testCases,
                               int minNumber);

    std::unique_ptr<SnippetRequest> createSnippetRequest(const std::string &projectName,
                                                         const fs::path &projectPath,
                                                         const fs::path &filePath);

    std::unique_ptr<ProjectRequest> createProjectRequest(const std::string &projectName,
                                                         const fs::path &projectPath,
                                                         const string &buildDirRelativePath,
                                                         const std::vector<fs::path> &srcPaths,
                                                         bool useStubs = false,
                                                         bool verbose = true,
                                                         int kleeTimeout = 30);

    std::unique_ptr<FileRequest> createFileRequest(const std::string &projectName,
                                                   const fs::path &projectPath,
                                                   const string &buildDirRelativePath,
                                                   const std::vector<fs::path> &srcPaths,
                                                   const fs::path &filePath,
                                                   bool useStubs = false);

    std::unique_ptr<LineRequest> createLineRequest(const std::string &projectName,
                                                   const fs::path &projectPath,
                                                   const string &buildDirRelativePath,
                                                   const std::vector<fs::path> &srcPaths,
                                                   const fs::path &filePath,
                                                   int line,
                                                   bool verbose = true,
                                                   int kleeTimeout = 30);

    std::unique_ptr<CoverageAndResultsRequest>
    createCoverageAndResultsRequest(const std::string &projectName,
                                    const fs::path &projectPath,
                                    const fs::path &testDirPath,
                                    const fs::path &buildDirRelativePath);

    std::unique_ptr<CoverageAndResultsRequest>
    createCoverageAndResultsRequest(const std::string &projectName,
                                    const fs::path &projectPath,
                                    const fs::path &testDirPath,
                                    const fs::path &buildDirRelativePath,
                                    std::unique_ptr<testsgen::TestFilter> testFilter);

    bool cmpChars(const std::string &charAsString, char c);

    void tryExecGetBuildCommands(
            const fs::path &path,
            CompilationUtils::CompilerName compilerName = CompilationUtils::CompilerName::CLANG,
            BuildCommandsTool buildCommandsTool = CMAKE_BUILD_COMMANDS_TOOL, bool build = true);

    fs::path getRelativeTestSuitePath(const string &suiteName);

    string fileNotExistsMessage(const fs::path &filePath);

    string unexpectedFileMessage(const fs::path &filePath);

    std::vector<char*> createArgvVector(const std::vector<std::string> &args);
}

#endif // UNITTESTBOT_TESTUTILS_H