#include "gtest/gtest.h"

#include "BaseTest.h"
#include "streams/stubs/StubsWriter.h"
#include "stubs/StubSourcesFinder.h"
#include "utils/FileSystemUtils.h"
#include "utils/path/FileSystemPath.h"
#include "coverage/CoverageAndResultsGenerator.h"
#include "streams/coverage/ServerCoverageAndResultsWriter.h"
#include "streams/stubs/ServerStubsWriter.h"
#include "stubs/StubGen.h"
#include "Synchronizer.h"

#include <fstream>

namespace {
    using testUtils::createFileRequest;
    using testUtils::createProjectRequest;

    class Stub_Test : public BaseTest {
    protected :
        Stub_Test() : BaseTest("stub") {}

        fs::path literals = getTestFilePath("lib/literals");
        fs::path calc = getTestFilePath("lib/calc");
        fs::path foreign = getTestFilePath("lib/foreign");
        fs::path function_pointers = getTestFilePath("lib/function_pointers");

        fs::path literals_foo_c = literals / "foo.c";
        fs::path calc_sum_c = calc / "sum.c";
        fs::path calc_sum_h = calc / "sum.h";
        fs::path calc_mult_c = calc / "mult.c";
        fs::path foreign_bar_c = foreign / "bar.c";
        fs::path function_pointers_c = function_pointers / "function_pointers.c";

        fs::path sum_stub_c = getTestFilePath(testDirName + "/stubs/lib/calc/sum_stub.c");

        fs::path testsDirPath = getTestFilePath("tests");
        utbot::ProjectContext projectContext{ projectName, suitePath, testsDirPath,
                                              buildDirRelativePath };
        fs::path sum_test_cpp =
                Paths::sourcePathToTestPath(projectContext, calc_sum_c);

        std::vector<fs::path> modifiedSourceFiles = { literals_foo_c, calc_sum_h, calc_sum_c };

        void SetUp() override {
            clearTestDirectory();
            clearEnv(CompilationUtils::CompilerName::CLANG);
            srcPaths = { suitePath, literals, calc };
            for (const auto& srcFilePath: modifiedSourceFiles) {
                FileSystemUtils::copyFile(suitePath / "original" / srcFilePath.filename(), srcFilePath);
            }
        }

        static void checkStubFilesExistence(const utbot::ProjectContext& projectContext, const std::vector<fs::path>& stubSourcePaths) {
            for (const auto& srcPath: stubSourcePaths) {
                fs::path stubFilePath = Paths::sourcePathToStubPath(projectContext, srcPath);
                EXPECT_TRUE(fs::exists(stubFilePath)) << testUtils::fileNotExistsMessage(stubFilePath);
            }
        }

        static void checkStubFileNoChanges(const fs::path& stubFilePath, const std::string& expectedContent) {
            std::ifstream stream(stubFilePath);
            std::string timestampComment;
            getline(stream, timestampComment);
            std::string fileContent((std::istreambuf_iterator<char>(stream)),
                                            std::istreambuf_iterator<char>());
            std::string expectedContentCopy = expectedContent;
            StringUtils::removeLineEndings(expectedContentCopy);
            StringUtils::removeLineEndings(fileContent);
            EXPECT_EQ(expectedContentCopy, fileContent);
        }

        static void checkStubFileEqualsTo(const fs::path& stubFilePath, const fs::path& expectedFilePath) {
            std::ifstream stream(stubFilePath);
            std::string timestampComment;
            getline(stream, timestampComment);
            std::string stubFileContent((std::istreambuf_iterator<char>(stream)),
                                    std::istreambuf_iterator<char>());
            std::ifstream expectedFileStream(expectedFilePath);
            std::string expectedFileContent((std::istreambuf_iterator<char>(expectedFileStream)),
                                        std::istreambuf_iterator<char>());
            StringUtils::removeLineEndings(stubFileContent);
            StringUtils::removeLineEndings(expectedFileContent);
            EXPECT_EQ(stubFileContent, expectedFileContent);
        }

        std::string modifyStubFile(const fs::path& stubFilePath) {
            fs::path modifiedContentPath = suitePath / "modified" / stubFilePath.filename();
            if (!fs::exists(stubFilePath) || !fs::exists(modifiedContentPath)) return "";
            std::ifstream modifiedIStream(modifiedContentPath);
            std::string modifiedFileContent((std::istreambuf_iterator<char>(modifiedIStream)),
                                            std::istreambuf_iterator<char>());

            std::ifstream stubIStream(stubFilePath);
            std::string timestampComment;
            getline(stubIStream, timestampComment);
            stubIStream.close();
            std::ofstream stubOStream(stubFilePath);
            std::string stubContent = timestampComment + modifiedFileContent;
            stubOStream << stubContent;
            auto fsNow = fs::file_time_type::clock::now();
            fs::last_write_time(stubFilePath, fsNow);
            return modifiedFileContent;
        }

        void modifySources(std::vector<fs::path>& sourceFiles) {
            for (const auto& srcPath: sourceFiles) {
                fs::path modifiedContentPath = suitePath / "modified" / srcPath.filename();
                ASSERT_TRUE(fs::exists(srcPath) && fs::exists(modifiedContentPath));
                FileSystemUtils::copyFile(modifiedContentPath, srcPath);
            }
        }
    };

    TEST_F(Stub_Test, Project_Stubs_Test) {
        auto stubsWriter = std::make_unique<ServerStubsWriter>(nullptr, false);
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
        auto testGen = std::make_unique<ProjectTestGen>(*request, writer.get(), TESTMODE);
        std::vector<fs::path> stubSources = {calc_sum_c, calc_mult_c, literals_foo_c};
        Status status =
            Server::TestsGenServiceImpl::ProcessProjectStubsRequest(testGen.get(), stubsWriter.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        checkStubFilesExistence(testGen->projectContext, stubSources);
    }

    TEST_F(Stub_Test, Implicit_Stubs_Test) {
        auto request = createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                         literals_foo_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkStubFilesExistence(testGen.projectContext, { calc_sum_c, calc_mult_c });
    }

    TEST_F(Stub_Test, Pregenerated_Stubs_Test) {
        {
            auto request = createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                             literals_foo_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
            auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();
        }
        std::string stubCode = modifyStubFile(sum_stub_c);

        {
            auto request = createFileRequest(projectName, suitePath, buildDirRelativePath,
                                              srcPaths, literals_foo_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
            auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();

            checkStubFileNoChanges(sum_stub_c, stubCode);
        }
    }

    TEST_F(Stub_Test, Multimodule_Lib_Heuristic_Test) {
        auto request = testUtils::createProjectRequest(projectName, suitePath, buildDirRelativePath,
                                                       {foreign, calc, suitePath, literals},
                                                       foreign_bar_c, true);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_EQ(testUtils::getNumberOfTests(testGen.tests), 2);

        const fs::path objectFile = testGen.getClientCompilationUnitInfo(foreign_bar_c)->getOutputFile();
        auto result = StubGen(testGen).getStubSetForObject(objectFile);
        ASSERT_TRUE(result.isSuccess());
        auto stubCandidates = {calc_sum_c};
        auto expectedStubFiles = CollectionUtils::transformTo<CollectionUtils::FileSet>(
                stubCandidates, [&testGen](fs::path const &path) {
                    return Paths::sourcePathToStubPath(testGen.projectContext, path);
                });
        EXPECT_EQ(expectedStubFiles, result.getOpt().value());
    }

    TEST_F(Stub_Test, File_Tests_With_Stubs) {
        auto request = testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath,
                                                    srcPaths, literals_foo_c, literals_foo_c, true);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_EQ(testUtils::getNumberOfTests(testGen.tests), 5);
        auto const &methods = testGen.tests.at(literals_foo_c).methods;
        testUtils::checkTestCasePredicates(
            methods.at("check_stubs").testCases,
            std::vector<TestCasePredicate>({
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "1";
                },
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "2";
                },
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "3";
                },
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "4";
                },
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "5";
                }}),
            "check_stubs");
    }

    TEST_F(Stub_Test, Run_Tests_For_Unused_Function) {
        auto request = testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath,
                                                   srcPaths, calc_sum_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_GE(testUtils::getNumberOfTests(testGen.tests), 2);

        auto testFilter = GrpcUtils::createTestFilterForFile(sum_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
            std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(), coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{ true, true, 15, 0, true, true, ErrorMode::FAILING };
        coverageGenerator.generate(true, settingsContext);
        EXPECT_FALSE(coverageGenerator.hasExceptions());
    }

    TEST_F(Stub_Test, File_Tests_Without_Stubs) {
        auto request = testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath,
                                                    srcPaths, literals_foo_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_EQ(testUtils::getNumberOfTests(testGen.tests), 4);
        auto const &methods = testGen.tests.at(literals_foo_c).methods;
        testUtils::checkTestCasePredicates(
            methods.at("check_stubs").testCases,
            std::vector<TestCasePredicate>({
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "1";
                },
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "2";
                },
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "3";
                },
                [](tests::Tests::MethodTestCase const &testCase) {
                    auto result = testCase.returnValue.view->getEntryValue(nullptr);
                    return result == "4";
                }}),
            "check_stubs");
    }

    TEST_F(Stub_Test, DISABLED_Sync_Stub_When_Source_Changed_Test) {
        auto request = createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths, literals_foo_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        std::vector<fs::path> sourcesToModify = {literals_foo_c, calc_sum_c, calc_sum_h};
        modifySources(sourcesToModify);
        std::string stubCode = modifyStubFile(sum_stub_c);

        auto request2 = createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths, literals_foo_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
        auto testGen2 = FileTestGen(*request2, writer.get(), TESTMODE);
        {
            auto request = createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                             literals_foo_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
            auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();
        }
        {
            auto request = createFileRequest(projectName, suitePath, buildDirRelativePath,
                                             srcPaths, literals_foo_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true);
            auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();

            checkStubFileEqualsTo(sum_stub_c, suitePath / "modified" / "sum_stub_sync.c");
        }
    }

    TEST_F(Stub_Test, Stubs_For_Function_Pointers) {
        auto request = createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths, function_pointers_c,
                                         GrpcUtils::UTBOT_AUTO_TARGET_PATH, true, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_EQ(testUtils::getNumberOfTests(testGen.tests), 7);

        fs::path testsDirPath = getTestFilePath("tests");

        fs::path function_pointers_test_cpp = Paths::sourcePathToTestPath(
                utbot::ProjectContext(projectName, suitePath, testsDirPath, buildDirRelativePath), function_pointers_c);
        auto testFilter = GrpcUtils::createTestFilterForFile(function_pointers_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
                projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
                std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(),
                                                       coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{
                true, false, 45, 30, false, true, ErrorMode::FAILING
        };
        coverageGenerator.generate(false, settingsContext);

        EXPECT_FALSE(coverageGenerator.hasExceptions());
        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        StatusCountMap expectedStatusCountMap{ {testsgen::TEST_PASSED, 7} };
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap);
    }
}
