#include "gtest/gtest.h"

#include "BaseTest.h"

namespace {
    using namespace testUtils;

    class BaseCMakeTest : public BaseTest {
    protected:
        BaseCMakeTest() : BaseTest("cmake") {}
        fs::path expectedCMakePath = suitePath / "expected_cmake.txt";

    public:
        void prepare(const std::string &subprojectName) {
            baseSuitePath /= "cmake";
            suiteName = subprojectName;
            clearEnv(CompilationUtils::CompilerName::CLANG); // will call setSuite(suiteName), which in turn will set suitePath and other paths
            expectedCMakePath = suitePath / "expected_cmake.txt";
        }

        void setExpectedCMakePath(const fs::path &path) {
            expectedCMakePath = path;
        }

        void checkCMakeGenerationForProjectTestgen(bool useStubs = false) {
            auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                                GrpcUtils::UTBOT_AUTO_TARGET_PATH, useStubs);
            auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();
            auto generatedCMake = FileSystemUtils::read(getTestDirectory() / "CMakeLists.txt");
            checkCMakeGenerated(suitePath, getTestDirectory(), expectedCMakePath);
        }

        void checkCMakeGenerationForFile(const fs::path &file, bool useStubs = false,
                                         const std::string &targetOrSource = GrpcUtils::UTBOT_AUTO_TARGET_PATH) {
            auto request = createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths, file,
                                             targetOrSource, useStubs);
            auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();
            auto generatedCMake = FileSystemUtils::read(getTestDirectory() / "CMakeLists.txt");
            checkCMakeGenerated(suitePath, getTestDirectory(), expectedCMakePath);
        }
    };

    TEST_F(BaseCMakeTest, simple_project
    ) {
        prepare("simple");
        checkCMakeGenerationForProjectTestgen();
    }

    TEST_F(BaseCMakeTest, project_with_shared_lib
    ) {
        prepare("shared");
        checkCMakeGenerationForProjectTestgen();
    }

    TEST_F(BaseCMakeTest, with_stubs) {
        prepare("stubs");

        setExpectedCMakePath(suitePath / "expected_cmake_exe_target.txt");
        checkCMakeGenerationForFile(getTestFilePath("unitA/caller.c"), true, "exe");

        setExpectedCMakePath(suitePath / "expected_cmake_unitA_target.txt");
        checkCMakeGenerationForFile(getTestFilePath("unitA/caller.c"), true, "unitA");
    }
}
