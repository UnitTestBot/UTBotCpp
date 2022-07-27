#ifndef UNITTESTBOT_TESTUTILS_H
#define UNITTESTBOT_TESTUTILS_H

#include "gtest/gtest.h"

#include "Server.h"
#include "Tests.h"
#include "coverage/Coverage.h"
#include "coverage/UnitTest.h"
#include "utils/CollectionUtils.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <regex>

using Params = const std::vector<std::shared_ptr<tests::AbstractValueView>> &;
using ReturnValue = const std::shared_ptr<tests::AbstractValueView> &;
using TestCasePredicate = std::function<bool(tests::Tests::MethodTestCase)>;
using CoverageLines = CollectionUtils::MapFileTo<std::set<int>>;
using StatusCountMap = std::unordered_map<testsgen::TestStatus, int>;

namespace testUtils {
    enum BuildCommandsTool {
        BEAR_BUILD_COMMANDS_TOOL,
        CMAKE_BUILD_COMMANDS_TOOL,
        MAKE_BUILD_COMMANDS_TOOL
    };

    void checkTestCasePredicates(const std::vector<tests::Tests::MethodTestCase> &testCases,
                                 const std::vector<TestCasePredicate> &predicates,
                                 const std::string &functionName = "");

    void checkRegexp(const std::string &value,
                     const std::string &regexp);

    void checkCoverage(const Coverage::CoverageMap &coverageMap,
                       const CoverageLines &expectedLinesCovered,
                       const CoverageLines &expectedLinesUncovered,
                       const CoverageLines &expectedLinesNone);

    void checkStatuses(const Coverage::TestResultMap &testResultMap, const std::vector<UnitTest> &tests);

    void checkStatusesCount(const Coverage::TestResultMap &testResultsMap,
                            const std::vector<UnitTest> &tests,
                            const StatusCountMap &expectedStatusCountMap);

    size_t getNumberOfTests(const tests::TestsMap &tests);

    size_t getNumberOfTestsForFile(const BaseTestGen &testGen, const std::string &fileName);

    void checkMinNumberOfTests(const tests::TestsMap &tests, size_t minNumber);

    void checkMinNumberOfTests(const std::vector<tests::Tests::MethodTestCase> &testCases,
                               size_t minNumber);

    void checkNumberOfTestsInFile(const BaseTestGen &testGen, std::string fileName, size_t number);

    void
    checkMinNumberOfTestsInFile(const BaseTestGen &testGen, std::string fileName, size_t number);

    std::unique_ptr<SnippetRequest> createSnippetRequest(const std::string &projectName,
                                                         const fs::path &projectPath,
                                                         const fs::path &filePath);

    std::unique_ptr<ProjectRequest> createProjectRequest(const std::string &projectName,
                                                         const fs::path &projectPath,
                                                         const std::string &buildDirRelativePath,
                                                         const std::vector<fs::path> &srcPaths,
                                                         const std::optional<std::string> &target = std::nullopt,
                                                         bool useStubs = false,
                                                         bool verbose = true,
                                                         int kleeTimeout = 60);

    std::unique_ptr<FileRequest> createFileRequest(const std::string &projectName,
                                                   const fs::path &projectPath,
                                                   const std::string &buildDirRelativePath,
                                                   const std::vector<fs::path> &srcPaths,
                                                   const fs::path &filePath,
                                                   const std::optional<std::string> &target = std::nullopt,
                                                   bool useStubs = false,
                                                   bool verbose = true,
                                                   int kleeTimeout = 60);

    std::unique_ptr<LineRequest> createLineRequest(const std::string &projectName,
                                                   const fs::path &projectPath,
                                                   const std::string &buildDirRelativePath,
                                                   const std::vector<fs::path> &srcPaths,
                                                   const fs::path &filePath,
                                                   int line,
                                                   const std::optional<std::string> &target = std::nullopt,
                                                   bool useStubs = false,
                                                   bool verbose = true,
                                                   int kleeTimeout = 60);

    std::unique_ptr<ClassRequest> createClassRequest(const std::string &projectName,
                                                     const fs::path &projectPath,
                                                     const std::string &buildDirRelativePath,
                                                     const std::vector<fs::path> &srcPaths,
                                                     const fs::path &filePath,
                                                     int line,
                                                     const std::optional<std::string> &target = std::nullopt,
                                                     bool useStubs = false,
                                                     bool verbose = true,
                                                     int kleeTimeout = 60);

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
            BuildCommandsTool buildCommandsTool = CMAKE_BUILD_COMMANDS_TOOL,
            bool build = true);

    fs::path getRelativeTestSuitePath(const std::string &suiteName);

    std::string fileNotExistsMessage(const fs::path &filePath);

    std::string unexpectedFileMessage(const fs::path &filePath);

    std::vector<char *> createArgvVector(const std::vector<std::string> &args);

    void setTargetForFirstSource(ProjectTestGen &testGen);

    void checkGenerationStatsCSV(const fs::path &statsPath, const std::vector<fs::path> &containedFiles);

    void checkExecutionStatsCSV(const fs::path &statsPath, const std::vector<fs::path> &containedFiles);
}

#endif // UNITTESTBOT_TESTUTILS_H
