#include "gtest/gtest.h"

#include "BaseTest.h"
#include "KleeGenerator.h"
#include "SettingsContext.h"

#include "utils/path/FileSystemPath.h"

namespace {
    using testsgen::TestsResponse;

    class KleeGen_Test : public BaseTest {
    protected:
        KleeGen_Test() : BaseTest("server") {}

        struct TestSuite {
            std::string name;
            fs::path buildPath;
            CollectionUtils::FileSet sourcesFilePaths;
        };

        fs::path tmpDirPath = baseSuitePath / buildDirRelativePath;
        TestSuite testSuite;

        void SetUp() override {
            clearEnv(CompilationUtils::CompilerName::CLANG);

            testSuite = { suiteName,
                          buildPath,
                          { getTestFilePath("assertion_failures.c"),
                            getTestFilePath("basic_functions.c"),
                            getTestFilePath("complex_structs.c"),
                            getTestFilePath("dependent_functions.c"),
                            getTestFilePath("external_dependent.c"),
                            getTestFilePath("floating_point.c"),
                            getTestFilePath("floating_point_plain.c"),
                            getTestFilePath("pointer_parameters.c"),
                            getTestFilePath("pointer_return.c"),
                            getTestFilePath("simple_structs.c"),
                            getTestFilePath("simple_unions.c"),
                            getTestFilePath("snippet.c"),
                            getTestFilePath("typedefs.c"),
                            getTestFilePath("types.c"),
                            getTestFilePath("inner/inner_basic_functions.c") } };
        }

        KleeGenerator initKleeGenerator(const TestSuite &suite, std::string &errorMessage) {
            std::shared_ptr<CompilationDatabase> compilationDatabase =
                CompilationDatabase::autoDetectFromDirectory(
                    suite.buildPath.string(), errorMessage);
            types::TypesHandler::SizeContext sizeContext;
            types::TypeMaps typeMaps;
            types::TypesHandler typesHandler(typeMaps, sizeContext);
            fs::path testsDirPath = tmpDirPath / "test";
            utbot::ProjectContext projectContext{ suite.name, "", testsDirPath,
                                                  buildDirRelativePath };
            auto buildDatabase = std::make_shared<BuildDatabase>(suite.buildPath, suite.buildPath, projectContext);
            utbot::SettingsContext settingsContext{ true, true, 15, 0, true, false };
            KleeGenerator generator(std::move(projectContext),
                                    std::move(settingsContext), tmpDirPath,
                                    compilationDatabase, typesHandler, {}, buildDatabase);
            return generator;
        }
    };

    TEST_F(KleeGen_Test, BuildByCDb) {
        std::string errorMessage;
        auto generator = initKleeGenerator(testSuite, errorMessage);
        ASSERT_TRUE(errorMessage.empty());
        CollectionUtils::FileSet sources(testSuite.sourcesFilePaths.begin(), testSuite.sourcesFilePaths.end());
        sources.erase(getTestFilePath("snippet.c"));
        sources.erase(getTestFilePath("external_dependent.c"));
        auto outFilesPaths = generator.buildByCDb(sources);
        for (const auto &[actualFilePath, srcFilePath] : outFilesPaths) {
            EXPECT_TRUE(fs::exists(actualFilePath)) << testUtils::fileNotExistsMessage(actualFilePath);
        }
    }

    TEST_F(KleeGen_Test, DefaultBuild) {
        std::string errorMessage;
        auto generator = initKleeGenerator(testSuite, errorMessage);
        ASSERT_TRUE(errorMessage.empty());
        fs::path sourceFilePath = *testSuite.sourcesFilePaths.begin();
        auto actualFilePath = generator.defaultBuild(sourceFilePath);
        EXPECT_TRUE(fs::exists(actualFilePath.getOpt().value()));
    }
}
