/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "TestUtils.h"

#include "environment/EnvironmentPaths.h"
#include "tasks/ShellExecTask.h"


namespace testUtils {
    using std::string;

    static string getMessageForTestCaseNotMatching(
        size_t predicateNumber,
        const string &functionName,
        const vector<vector<shared_ptr<tests::AbstractValueView>>> &parameters,
        const vector<shared_ptr<tests::AbstractValueView>> &returnValues) {
        std::stringstream ss;
        ss << "Predicates don't match test cases:\n";
        ss << "\tNot found test case for predicate at position:" << predicateNumber << "\n";
        ss << "\tFunction name: " << functionName << "\n";
        ss << "\tRemaining non-matched test cases:\n";
        for (size_t i = 0; i < parameters.size(); i++) {
            ss << "\t\tParameters values: ";
            for (const auto &param : parameters[i]) {
                ss << param->getEntryValue() << " ";
            }
            ss << "\n\t\tReturn value: " << returnValues[i]->getEntryValue() << "\n";
        }
        return ss.str();
    }

    void checkTestCasePredicates(const vector<tests::Tests::MethodTestCase> &testCases,
                                 const vector<TestCasePredicate> &predicates,
                                 const string &functionName) {
        EXPECT_GE(testCases.size(), predicates.size())
            << " Number of test cases (" << testCases.size()
            << ") less than"
               " number of predicates ("
            << predicates.size() << ") for function " << functionName << ".";
        vector<vector<shared_ptr<tests::AbstractValueView>>> params;
        vector<shared_ptr<tests::AbstractValueView>> returnValues;
        for (const auto &testCase : testCases) {
            params.emplace_back();
            for (const auto &p : testCase.paramValues) {
                params.back().push_back(p.view);
            }
            returnValues.push_back(testCase.returnValueView);
        }
        for (size_t p_i = 0; p_i < predicates.size(); p_i++) {
            const auto &predicate = predicates[p_i];
            int ind = -1;
            for (size_t i = 0; i < testCases.size(); i++) {
                if (predicate(testCases[i])) {
                    ind = i;
                    break;
                }
            }
            EXPECT_NE(-1, ind) << getMessageForTestCaseNotMatching(p_i, functionName, params,
                                                                   returnValues);
            if (ind == -1) {
                return;
            }
            params.erase(params.begin() + ind);
            returnValues.erase(returnValues.begin() + ind);
        }
    }

    void checkCoverage(const Coverage::CoverageMap &coverageMap,
                       const CoverageLines &expectedLinesCovered,
                       const CoverageLines &expectedLinesUncovered,
                       const CoverageLines &expectedLinesNone) {
        CoverageLines actualLinesCovered;
        CoverageLines actualLinesUncovered;
        for (const auto &[filePath, fileCoverage] : coverageMap) {
            for(const auto &sourceLine : fileCoverage.fullCoverageLines) {
                actualLinesCovered[filePath].insert(sourceLine.line);
            }
            for(const auto &sourceLine : fileCoverage.noCoverageLines) {
                actualLinesUncovered[filePath].insert(sourceLine.line);
            }
        }
        for (const auto &[filePath, fileCoverage] : expectedLinesCovered) {
            auto actualLinesCoveredForFile = actualLinesCovered[filePath];
            EXPECT_TRUE(std::includes(actualLinesCoveredForFile.begin(),
                                      actualLinesCoveredForFile.end(), fileCoverage.begin(),
                                      fileCoverage.end()))
                << StringUtils::stringFormat("Expected covered lines for %s are not included\n"
                                             "Actual lines covered: %s\n"
                                             "File coverage: %s\n",
                                             filePath,
                                             StringUtils::joinWith(actualLinesCoveredForFile, " "),
                                             StringUtils::joinWith(fileCoverage, " "));
        }
        for (const auto &[filePath, fileCoverage] : expectedLinesUncovered) {
            auto actualLinesUnCoveredForFile = actualLinesUncovered[filePath];
            EXPECT_TRUE(std::includes(actualLinesUnCoveredForFile.begin(),
                                      actualLinesUnCoveredForFile.end(), fileCoverage.begin(),
                                      fileCoverage.end()))
                << StringUtils::stringFormat(
                       "Expected uncovered lines for %s are not included\n"
                       "Actual lines uncovered: %s\n"
                       "File uncoverage: %s\n",
                       filePath, StringUtils::joinWith(actualLinesUnCoveredForFile, " "),
                       StringUtils::joinWith(fileCoverage, " "), filePath);
        }
        for (const auto &[filePath, fileCoverage] : expectedLinesNone) {
            {
                std::vector<int> intersectionWithUncovered;
                std::set_intersection(actualLinesUncovered[filePath].begin(),
                                      actualLinesUncovered[filePath].end(), fileCoverage.begin(),
                                      fileCoverage.end(),
                                      std::back_inserter(intersectionWithUncovered));
                EXPECT_TRUE(intersectionWithUncovered.empty()) << StringUtils::stringFormat(
                    "Expected none lines for %s are uncovered\n"
                    "Lines: %s",
                    filePath, StringUtils::joinWith(intersectionWithUncovered, " "));
            }
            {
                std::vector<int> intersectionWithCovered;
                std::set_intersection(actualLinesCovered[filePath].begin(),
                                      actualLinesCovered[filePath].end(), fileCoverage.begin(),
                                      fileCoverage.end(),
                                      std::back_inserter(intersectionWithCovered));
                EXPECT_TRUE(intersectionWithCovered.empty()) << StringUtils::stringFormat(
                    "Expected none lines for %s are covered\n"
                    "Lines: %s",
                    filePath, StringUtils::joinWith(intersectionWithCovered, " "));
            }
        }
    }

    void checkStatuses(const Coverage::TestStatusMap &testStatusMap,
                       const vector<UnitTest> &tests) {
        for (auto const &[filename, suitename, testname] : tests) {
            const auto status = testStatusMap.at(filename).at(testname);
            // ToDo:
//            if (suitename == tests::Tests::ERROR_SUITE_NAME) {
//                EXPECT_TRUE((testsgen::TestStatus::TEST_FAILED == status) ||
//                (testsgen::TestStatus::TEST_DEATH == status));
//            } else {
//                EXPECT_TRUE((testsgen::TestStatus::TEST_PASSED == status) ||
//                (testsgen::TestStatus::TEST_DEATH == status));
//            }
            EXPECT_TRUE((testsgen::TestStatus::TEST_PASSED == status) ||
                        (testsgen::TestStatus::TEST_FAILED == status) ||
                        testsgen::TestStatus::TEST_DEATH == status);
        }
    }

    int getNumberOfTests(const tests::TestsMap &tests) {
        auto testFilePaths = CollectionUtils::getKeys(tests);
        int testsCounter = 0;
        for (const auto &[filename, cases] : tests) {
            for (const auto &[_, methodDescription] : cases.methods) {
                int size = methodDescription.testCases.size();
                testsCounter += size;
            }
        }
        return testsCounter;
    }

    void checkMinNumberOfTests(const tests::TestsMap &tests, int minNumber) {
        int testsCounter = getNumberOfTests(tests);
        EXPECT_LE(minNumber, testsCounter) << "Number of test cases is too small";
    }

    void checkMinNumberOfTests(const vector<tests::Tests::MethodTestCase> &testCases,
                               int minNumber) {
        EXPECT_LE(minNumber, testCases.size()) << "Number of test cases is too small";
    }

    std::unique_ptr<ProjectRequest> createProjectRequest(const std::string &projectName,
                                                         const fs::path &projectPath,
                                                         const string &buildDirRelativePath,
                                                         const std::vector<fs::path> &srcPaths,
                                                         bool useStubs,
                                                         bool verbose,
                                                         int kleeTimeout) {
        auto projectContext = GrpcUtils::createProjectContext(
            projectName, projectPath, projectPath / "tests", buildDirRelativePath);
        auto settingsContext = GrpcUtils::createSettingsContext(true, verbose, kleeTimeout, 0, false, useStubs);
        return GrpcUtils::createProjectRequest(std::move(projectContext),
                                               std::move(settingsContext), srcPaths);
    }

    std::unique_ptr<FileRequest> createFileRequest(const std::string &projectName,
                                                   const fs::path &projectPath,
                                                   const string &buildDirRelativePath,
                                                   const std::vector<fs::path> &srcPaths,
                                                   const fs::path &filePath,
                                                   bool useStubs,
                                                   bool verbose) {
        auto projectRequest = createProjectRequest(projectName, projectPath, buildDirRelativePath,
                                                   srcPaths, useStubs, verbose);
        return GrpcUtils::createFileRequest(std::move(projectRequest), filePath);
    }

    std::unique_ptr<LineRequest> createLineRequest(const std::string &projectName,
                                                   const fs::path &projectPath,
                                                   const string &buildDirRelativePath,
                                                   const std::vector<fs::path> &srcPaths,
                                                   const fs::path &filePath,
                                                   int line,
                                                   bool verbose,
                                                   int kleeTimeout) {
        auto projectRequest = createProjectRequest(projectName, projectPath, buildDirRelativePath,
                                                   srcPaths, false, verbose, kleeTimeout);
        auto lineInfo = GrpcUtils::createSourceInfo(filePath, line);
        return GrpcUtils::createLineRequest(std::move(projectRequest), std::move(lineInfo));
    }

    std::unique_ptr<ClassRequest> createClassRequest(const std::string &projectName,
                                                   const fs::path &projectPath,
                                                   const string &buildDirRelativePath,
                                                   const std::vector<fs::path> &srcPaths,
                                                   const fs::path &filePath,
                                                   int line,
                                                   bool verbose,
                                                   int kleeTimeout) {
        auto lineRequest = createLineRequest(projectName, projectPath, buildDirRelativePath,
                                             srcPaths, filePath, line, verbose, kleeTimeout);
        return GrpcUtils::createClassRequest(std::move(lineRequest));
    }

    std::unique_ptr<SnippetRequest> createSnippetRequest(const std::string &projectName,
                                                         const fs::path &projectPath,
                                                         const fs::path &filePath) {
        auto projectContext =
            GrpcUtils::createProjectContext(projectName, projectPath, projectPath / "tests", "");
        // we actually don't pass all parameters except test directory and project name on client
        auto settingsContext = GrpcUtils::createSettingsContext(true, true, 10, 0, true, false);
        return GrpcUtils::createSnippetRequest(std::move(projectContext),
                                               std::move(settingsContext), filePath);
    }

    std::unique_ptr<CoverageAndResultsRequest>
    createCoverageAndResultsRequest(const std::string &projectName,
                                    const fs::path &projectPath,
                                    const fs::path &testDirPath,
                                    const fs::path &buildDirRelativePath,
                                    std::unique_ptr<testsgen::TestFilter> testFilter) {
        auto request = std::make_unique<CoverageAndResultsRequest>();
        auto projectContext = GrpcUtils::createProjectContext(projectName, projectPath, testDirPath,
                                                              buildDirRelativePath);
        request->set_allocated_projectcontext(projectContext.release());
        request->set_allocated_testfilter(testFilter.release());
        request->set_coverage(true);
        return request;
    }

    bool cmpChars(const string &charAsString, char c) {
        return charAsString ==
               std::string("'") + StringUtils::charCodeToLiteral(c) + std::string("'");
    }

    static std::string setupCompilerCommand(CompilationUtils::CompilerName compilerName) {
        switch (compilerName) {
        case CompilationUtils::CompilerName::GCC:
            return StringUtils::stringFormat(
                "rm -rf build_gcc && mkdir -p build_gcc && cd build_gcc && "
                "export CC=%s && export CXX=%s && export C_INCLUDE_PATH=$UTBOT_LAUNCH_INCLUDE_PATH",
                Paths::getGcc(), Paths::getGpp());
        case CompilationUtils::CompilerName::CLANG:
            return StringUtils::stringFormat(
                "rm -rf build_clang && mkdir -p build_clang && cd build_clang && "
                "export CC=%s && export CXX=%s",
                Paths::getUTBotClang(), Paths::getUTBotClangPP());
        default:
            throw CompilationDatabaseException("Test build not implemented for current compiler");
        }
    }

    static std::string setupGettingBuildCommands(BuildCommandsTool buildCommandsTool,
                                                 CompilationUtils::CompilerName compilerName,
                                                 bool build) {
        std::string result;
        string interceptor;
        switch (buildCommandsTool) {
        case BuildCommandsTool::CMAKE_BUILD_COMMANDS_TOOL:
            result += StringUtils::stringFormat(
                "%s -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_EXPORT_LINK_COMMANDS=ON ..",
                Paths::getCMake());
            interceptor = "";
            break;
        case BuildCommandsTool::BEAR_BUILD_COMMANDS_TOOL:
            result += StringUtils::stringFormat("%s ..", Paths::getCMake());
            interceptor = Paths::getBear();
            break;
        case BuildCommandsTool::MAKE_BUILD_COMMANDS_TOOL:
            result += "cd ..";
            interceptor = Paths::getBear();
            break;
        default:
            throw CompilationDatabaseException(
                "Test build not implemented for current build commands tool");
        }
        if (build) {
            result += StringUtils::stringFormat(" && %s %s -j8", interceptor, Paths::getMake());
            if (buildCommandsTool == BuildCommandsTool::MAKE_BUILD_COMMANDS_TOOL) {
                string buildDir = (compilerName == CompilationUtils::CompilerName::GCC)
                                      ? "build_gcc"
                                      : "build_clang";
                auto copyJsonIntoBuildDir = [&](const string &buildDir, const string &jsonName) {
                    return StringUtils::stringFormat("cp %s.json %s/%s.json", jsonName, buildDir,
                                                     jsonName);
                };
                result += StringUtils::stringFormat(
                    "&& %s && %s", copyJsonIntoBuildDir(buildDir, "compile_commands"),
                    copyJsonIntoBuildDir(buildDir, "link_commands"));
            }
        }
        return result;
    }

    void tryExecGetBuildCommands(const fs::path &path,
                                 CompilationUtils::CompilerName compilerName,
                                 BuildCommandsTool buildCommandsTool,
                                 bool build) {
        string command = setupCompilerCommand(compilerName) + " && " +
                         setupGettingBuildCommands(buildCommandsTool, compilerName, build);

        auto res =
            ShellExecTask::runPlainShellCommand(command, fs::current_path().parent_path() / path);
        if (res.status != 0) {
            LOG_S(ERROR) << "Build of " << path << " failed\n" << res.output;
            LOG_S(ERROR) << "With command: " << command;
            throw std::runtime_error("exec failed");
        }
    }

    fs::path getRelativeTestSuitePath(const string &suiteName) {
        return fs::path("test") / "suites" / suiteName;
    }

    string fileNotExistsMessage(const fs::path &filePath) {
        return "Expected existence of this file:\n\t" + filePath.string();
    }

    string unexpectedFileMessage(const fs::path &filePath) {
        return "Unexpected file found:\n\t" + filePath.string();
    }

    std::vector<char *> createArgvVector(const vector<std::string> &args) {
        auto argv = CollectionUtils::transformTo<std::vector<char *>>(
            args, [](const std::string &arg) { return (char *)arg.data(); });
        argv.push_back(nullptr);

        return argv;
    }

}
