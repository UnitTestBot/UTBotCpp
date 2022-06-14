#include "TestRunner.h"

#include "GTestLogger.h"
#include "Paths.h"
#include "TimeExecStatistics.h"
#include "utils/FileSystemUtils.h"
#include "utils/StringUtils.h"

#include "loguru.h"

using grpc::ServerWriter;
using grpc::Status;

TestRunner::TestRunner(utbot::ProjectContext projectContext,
                       std::string testFilePath,
                       std::string testSuite,
                       std::string testName,
                       ProgressWriter const *progressWriter)
    : projectContext(std::move(projectContext)),
      testFilePath(testFilePath.empty() ? std::nullopt : std::make_optional(testFilePath)),
      testSuite(std::move(testSuite)), testName(std::move(testName)),
      progressWriter(progressWriter) {
}

TestRunner::TestRunner(
    const testsgen::CoverageAndResultsRequest *coverageAndResultsRequest,
    grpc::ServerWriter<testsgen::CoverageAndResultsResponse> *coverageAndResultsWriter,
    std::string testFilename,
    std::string testSuite,
    std::string testName)
    : TestRunner(utbot::ProjectContext(coverageAndResultsRequest->projectcontext()),
                 std::move(testFilename),
                 std::move(testSuite),
                 std::move(testName),
                 &writer) {
    writer = ServerCoverageAndResultsWriter(coverageAndResultsWriter);
}

std::vector<UnitTest> TestRunner::getTestsFromMakefile(const fs::path &makefile,
                                                       const fs::path &testFilePath) {
    auto cmdGetAllTests = MakefileUtils::makefileCommand(projectContext, makefile, "run", "--gtest_list_tests", {"GTEST_FILTER=*"});
    auto [out, status, _] = cmdGetAllTests.run(projectContext.buildDir, false);
    if (status != 0) {
        auto [err, _, logFilePath] = cmdGetAllTests.run(projectContext.buildDir, true);
        progressWriter->writeProgress(StringUtils::stringFormat("command %s failed.\n"
                                                                "see: \"%s\"",
                                                                cmdGetAllTests.getFailedCommand(),
                                                                logFilePath.value()));

        throw ExecutionProcessException(err, logFilePath.value());
    }
    if (out.empty()) {
        LOG_S(WARNING) << "Running gtest with flag --gtest_list_tests returns empty output. Does file contain main function?";
        return {};
    }
    std::vector<std::string> gtestListTestsOutput = StringUtils::split(out, '\n');
    gtestListTestsOutput.erase(gtestListTestsOutput.begin()); //GTEST prints "Running main() from /opt/gtest/googletest/src/gtest_main.cc"
    for (std::string &s : gtestListTestsOutput) {
        StringUtils::trim(s);
    }
    std::string testSuite;
    std::vector<std::string> testsList;
    for (const std::string &s : gtestListTestsOutput) {
        if (s.back() == '.') {
            testSuite = s;
            testSuite.pop_back();
        } else {
            testsList.push_back(s);
        }
    }
    return CollectionUtils::transform(testsList, [&testFilePath, &testSuite](std::string const &name) {
        return UnitTest{ testFilePath, testSuite, name };
    });
}

std::vector<UnitTest> TestRunner::getTestsToLaunch() {
    if (!testFilePath.has_value()) {
        //for project
        std::vector<UnitTest> result;

        if (fs::exists(projectContext.testDirPath)) {
            FileSystemUtils::RecursiveDirectoryIterator directoryIterator(projectContext.testDirPath);
            ExecUtils::doWorkWithProgress(
                directoryIterator, progressWriter, "Building tests",
                [this, &result](fs::directory_entry const &directoryEntry) {
                    if (!directoryEntry.is_regular_file()) {
                        return;
                    }
                    const auto &testFilePath = directoryEntry.path();
                    if (StringUtils::endsWith(testFilePath.c_str(), "_test.cpp")) {
                        fs::path sourcePath = Paths::testPathToSourcePath(projectContext, testFilePath);
                        fs::path makefile =
                            Paths::getMakefilePathFromSourceFilePath(projectContext, sourcePath);
                        if (fs::exists(makefile)) {
                            try {
                                auto tests = getTestsFromMakefile(makefile, testFilePath);
                                CollectionUtils::extend(result, tests);
                            } catch (ExecutionProcessException const &e) {
                                exceptions.push_back(e);
                            }
                        } else {
                            LOG_S(WARNING) << StringUtils::stringFormat(
                                "Makefile for %s not found, candidate: %s", testFilePath, makefile);
                        }
                    } else {
                        if (!StringUtils::endsWith(testFilePath.c_str(), "_test.h") &&
                            !StringUtils::endsWith(testFilePath.stem().c_str(), "_stub")) {
                            LOG_S(WARNING)
                                << "Found extra file in test directory: " << testFilePath;
                        }
                    }
                });
        } else {
            LOG_S(WARNING) << "Test folder doesn't exist: " << projectContext.testDirPath;
        }
        return result;
    }
    if (testName.empty()) {
        //for file
        fs::path sourcePath = Paths::testPathToSourcePath(projectContext, testFilePath.value());
        fs::path makefile = Paths::getMakefilePathFromSourceFilePath(projectContext, sourcePath);
        return getTestsFromMakefile(makefile, testFilePath.value());
    }
    //for single test
    return { UnitTest{ testFilePath.value(), testSuite, testName } };
}

grpc::Status TestRunner::runTests(bool withCoverage, const std::optional<std::chrono::seconds> &testTimeout) {
    MEASURE_FUNCTION_EXECUTION_TIME
    ExecUtils::throwIfCancelled();

    const auto buildRunCommands = coverageTool->getBuildRunCommands(testsToLaunch, withCoverage);
    ExecUtils::doWorkWithProgress(buildRunCommands, progressWriter, "Running tests",
                              [this, testTimeout] (BuildRunCommand const &buildRunCommand) {
                                  auto const &[unitTest, buildCommand, runCommand] =
                                      buildRunCommand;
                                  try {
                                      auto status = runTest(runCommand, testTimeout);
                                      testStatusMap[unitTest.testFilePath][unitTest.testname] = status;
                                      ExecUtils::throwIfCancelled();
                                  } catch (ExecutionProcessException const &e) {
                                      testStatusMap[unitTest.testFilePath][unitTest.testname] = testsgen::TEST_FAILED;
                                      exceptions.emplace_back(e);
                                  }
                              });

    LOG_S(DEBUG) << "All run commands were executed";
    return Status::OK;
}

void TestRunner::init(bool withCoverage) {
    MEASURE_FUNCTION_EXECUTION_TIME
    fs::path ccJson = CompilationUtils::substituteRemotePathToCompileCommandsJsonPath(
        projectContext.projectPath, projectContext.buildDirRelativePath);
    coverageTool = getCoverageTool(ccJson, projectContext, progressWriter);
    if (withCoverage) {
        cleanCoverage();
    }
    testsToLaunch = getTestsToLaunch();
    if (withCoverage) {
        cleanCoverage();
    }
}

bool TestRunner::buildTest(const utbot::ProjectContext& projectContext, const fs::path& sourcePath) {
    ExecUtils::throwIfCancelled();
    fs::path makefile = Paths::getMakefilePathFromSourceFilePath(projectContext, sourcePath);
    if (fs::exists(makefile)) {
        auto command = MakefileUtils::makefileCommand(projectContext, makefile, "build", "", {});
        LOG_S(DEBUG) << "Try compile tests for: " << sourcePath.string();
        auto[out, status, logFilePath] = command.run(projectContext.buildDir, true);
        if (status != 0) {
            return false;
        }
        return true;
    }
    return false;
}

size_t TestRunner::buildTests(const utbot::ProjectContext& projectContext, const tests::TestsMap& tests) {
    size_t fail_count = 0;
    for (const auto &[file, _]: tests) {
        if(!TestRunner::buildTest(projectContext, file)) {
            fail_count++;
        }
    }
    return fail_count;
}

testsgen::TestStatus TestRunner::runTest(const MakefileUtils::MakefileCommand &command, const std::optional <std::chrono::seconds> &testTimeout) {
    auto res = command.run(projectContext.buildDir, true, true, testTimeout);
    GTestLogger::log(res.output);
    if (StringUtils::contains(res.output, "[  PASSED  ] 1 test")) {
        return testsgen::TEST_PASSED;
    }
    if (StringUtils::contains(res.output, "[  FAILED  ] 1 test")) {
        return testsgen::TEST_FAILED;
    }
    if (BaseForkTask::wasInterrupted(res.status)) {
        return testsgen::TEST_INTERRUPTED;
    }
    return testsgen::TEST_DEATH;
}

const Coverage::TestStatusMap &TestRunner::getTestStatusMap() const {
    return testStatusMap;
}

bool TestRunner::hasExceptions() const {
    return !exceptions.empty();
}

void TestRunner::cleanCoverage() {
    ExecUtils::throwIfCancelled();

    coverageTool->cleanCoverage();
}