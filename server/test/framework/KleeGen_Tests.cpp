/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "gtest/gtest.h"

#include "BaseTest.h"
#include "KleeGenerator.h"
#include "SettingsContext.h"
#include "building/Linker.h"

#include "utils/path/FileSystemPath.h"

namespace {
    using namespace std;
    using testsgen::TestsResponse;
    using namespace clang::tooling;

    class KleeGen_Test : public BaseTest {
    protected:
        KleeGen_Test() : BaseTest("server") {}

        struct TestSuite {
            string name;
            fs::path buildPath;
            vector<fs::path> sourcesFilePaths;
        };

        fs::path tmpDirPath = baseSuitePath / buildDirRelativePath;
        TestSuite testSuite;

        void SetUp() override {
            clearEnv();

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

        KleeGenerator initKleeGenerator(const TestSuite &suite, string &errorMessage) {
            std::shared_ptr<clang::tooling::CompilationDatabase> compilationDatabase =
                CompilationDatabase::autoDetectFromDirectory(suite.buildPath.string(),
                                                             errorMessage);
            types::TypesHandler::SizeContext sizeContext;
            types::TypeMaps typeMaps;
            types::TypesHandler typesHandler(typeMaps, sizeContext);
            fs::path testsDirPath = tmpDirPath / "test";
            utbot::ProjectContext projectContext{ suite.name, "", testsDirPath,
                                                  buildDirRelativePath };
            auto buildDatabase =
                std::make_shared<BuildDatabase>(suite.buildPath, suite.buildPath, projectContext);
            utbot::SettingsContext settingsContext{ true, true, 15, 0, true, false };
            KleeGenerator generator(std::move(projectContext),
                                    std::move(settingsContext), tmpDirPath, suite.sourcesFilePaths,
                                    compilationDatabase, typesHandler, {}, buildDatabase);
            return generator;
        }
    };

    TEST_F(KleeGen_Test, BuildByCDb) {
        string errorMessage;
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
        string errorMessage;
        auto generator = initKleeGenerator(testSuite, errorMessage);
        ASSERT_TRUE(errorMessage.empty());
        auto sourceFilePath = testSuite.sourcesFilePaths[0];
        auto actualFilePath = generator.defaultBuild(sourceFilePath);
        EXPECT_TRUE(fs::exists(actualFilePath.getOpt().value()));
    }
}