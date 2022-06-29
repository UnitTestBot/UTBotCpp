#ifndef UNITTESTBOT_TESTRUNNER_H
#define UNITTESTBOT_TESTRUNNER_H

#include "CoverageTool.h"
#include "ProjectContext.h"
#include "UnitTest.h"
#include "exceptions/ExecutionProcessException.h"
#include "utils/path/FileSystemPath.h"
#include "streams/IStreamWriter.h"
#include "streams/coverage/CoverageAndResultsWriter.h"
#include "streams/coverage/ServerCoverageAndResultsWriter.h"
#include "Tests.h"

#include <string>
#include <vector>

class TestRunner {
protected:
    const utbot::ProjectContext projectContext;
    const std::optional<fs::path> testFilePath;
    const std::string testSuite;
    const std::string testName;
    ProgressWriter const *progressWriter;

    std::unique_ptr<CoverageTool> coverageTool{};
    std::vector<UnitTest> testsToLaunch{};
    Coverage::TestResultMap testResultMap{};

    std::vector<ExecutionProcessException> exceptions;

    grpc::Status runTests(bool withCoverage,
                          const std::optional<std::chrono::seconds> &testTimeout);

public:
    TestRunner(utbot::ProjectContext projectContext,
               std::string testFilePath,
               std::string testSuite,
               std::string testName,
               ProgressWriter const *progressWriter);

    TestRunner(const testsgen::CoverageAndResultsRequest *coverageAndResultsRequest,
               grpc::ServerWriter<testsgen::CoverageAndResultsResponse> *coverageAndResultsWriter,
               std::string testFilename,
               std::string testSuite,
               std::string testName);

    void init(bool withCoverage);

    std::vector<UnitTest> getTestsToLaunch();

    const Coverage::TestResultMap &getTestResultMap() const;

    bool hasExceptions() const;


    /**
     * Try compile test for source file and return true if succeed, else false
     */
    static bool buildTest(const utbot::ProjectContext& projectContext, const fs::path& sourcePath);


    /**
     * Try compile tests for files in tests and return count of failed attempts
     */
    static size_t buildTests(const utbot::ProjectContext& projectContext, const tests::TestsMap& tests);

private:
    std::vector<UnitTest> getTestsFromMakefile(const fs::path &makefile,
                                               const fs::path &testFilePath);

    testsgen::TestResultObject runTest(const BuildRunCommand &command,
                                       const std::optional<std::chrono::seconds> &testTimeout);

    ServerCoverageAndResultsWriter writer{ nullptr };

    void cleanCoverage();
};


#endif // UNITTESTBOT_TESTRUNNER_H
