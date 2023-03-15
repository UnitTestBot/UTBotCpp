#include "gtest/gtest.h"

#include "BaseTest.h"
#include "KleeGenerator.h"
#include "ProjectContext.h"
#include "Server.h"
#include "clang-utils/SourceToHeaderRewriter.h"
#include "coverage/CoverageAndResultsGenerator.h"
#include "printers/HeaderPrinter.h"
#include "printers/TestMakefilesPrinter.h"
#include "printers/SourceWrapperPrinter.h"
#include "utils/FileSystemUtils.h"
#include "utils/ServerUtils.h"

#include "utils/path/FileSystemPath.h"
#include <functional>
#include <tuple>

namespace {
    using CompilationUtils::CompilerName;
    using CompilationUtils::getBuildDirectoryName;
    using grpc::Channel;
    using grpc::ClientContext;
    using testsgen::TestsGenService;
    using testsgen::TestsResponse;
    using namespace testUtils;

    class Server_Test : public BaseTest {
    protected:
        Server_Test() : BaseTest("server") {
        }

        fs::path snippet_c = getTestFilePath("snippet.c");
        fs::path assertion_failures_c = getTestFilePath("assertion_failures.c");
        fs::path basic_functions_c = getTestFilePath("basic_functions.c");
        fs::path dependent_functions_c = getTestFilePath("dependent_functions.c");
        fs::path different_variables_c = getTestFilePath("different_variables.c");
        fs::path pointer_parameters_c = getTestFilePath("pointer_parameters.c");
        fs::path simple_structs_c = getTestFilePath("simple_structs.c");
        fs::path simple_unions_c = getTestFilePath("simple_unions.c");
        fs::path types_c = getTestFilePath("types.c");
        fs::path inner_basic_functions_c = getTestFilePath("inner/inner_basic_functions.c");
        fs::path pointer_return_c = getTestFilePath("pointer_return.c");
        fs::path floating_point_c = getTestFilePath("floating_point.c");
        fs::path floating_point_plain_c = getTestFilePath("floating_point_plain.c");
        fs::path linkage_c = getTestFilePath("linkage.c");
        fs::path globals_c = getTestFilePath("globals.c");
        fs::path keywords_c = getTestFilePath("keywords.c");
        fs::path alignment_c = getTestFilePath("alignment.c");
        fs::path symbolic_stdin_c = getTestFilePath("symbolic_stdin.c");
        fs::path multiple_classes_h = getTestFilePath("multiple_classes.h");
        fs::path multiple_classes_cpp = getTestFilePath("multiple_classes.cpp");

        void SetUp() override {
            clearEnv(CompilationUtils::CompilerName::CLANG);
        }

        void generateFiles(const fs::path &sourceFile, const fs::path &testsRelativeDir) {
            fs::path testsDirPath = getTestFilePath(testsRelativeDir);

            auto projectContext = GrpcUtils::createProjectContext(
                    projectName, suitePath, testsDirPath, buildDirRelativePath);

            auto settingsContext = GrpcUtils::createSettingsContext(true, false, 30, 0, false, false, ErrorMode::PASSING, false);

            auto request = GrpcUtils::createProjectRequest(std::move(projectContext),
                                                           std::move(settingsContext),
                                                           srcPaths,
                                                           sourceFile);

            auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

            generateFiles(sourceFile, testGen);
        }

        void generateFiles(const fs::path &sourceFile, const BaseTestGen& testGen) {
            fs::path serverBuildDir = buildPath / "temp";
            fs::path compilerPath = CompilationUtils::getBundledCompilerPath(compilerName);
            CollectionUtils::FileSet stubsSources;
            fs::path root = testGen.getTargetBuildDatabase()->getTargetPath();
            printer::TestMakefilesPrinter testMakefilePrinter(&testGen, root, compilerPath, &stubsSources);
            testMakefilePrinter.addLinkTargetRecursively(root, "");

            testMakefilePrinter.GetMakefiles(sourceFile).write();

            auto compilationDatabase = CompilationUtils::getCompilationDatabase(buildPath);
            auto structsToDeclare = std::make_shared<Fetcher::FileToStringSet>();
            SourceToHeaderRewriter sourceToHeaderRewriter(testGen.projectContext, compilationDatabase,
                                                          structsToDeclare, serverBuildDir);
            std::string wrapper = sourceToHeaderRewriter.generateWrapper(sourceFile);
            printer::SourceWrapperPrinter(Paths::getSourceLanguage(sourceFile)).print(testGen.projectContext,
                                                                                      sourceFile, wrapper);
        }


        void generateMakefilesForProject(const fs::path &testsRelativeDir) {
            for (const auto &source : fs::directory_iterator(suitePath)) {
                const fs::path& sourceFilePath = source.path();
                if (Paths::isCFile(sourceFilePath)) {
                    generateFiles(sourceFilePath, testsRelativeDir);
                }
            }
        }

        struct FileGenResult {
            FileTestGen testGen;
            Status status;

            FileGenResult() = delete;

            FileGenResult(FileTestGen ttestGen, Status status) : testGen(std::move(ttestGen)),
                                                                status(std::move(status)) {}
        };

        FileGenResult performFeatureFileTestsRequest(const fs::path &filename) {
            auto projectRequest =
                    createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths);
            auto request = GrpcUtils::createFileRequest(std::move(projectRequest), filename);
            auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            return FileGenResult(testGen, status);
        }

        void checkAssertionFailures_C(BaseTestGen &testGen) {
            testUtils::checkTestCasePredicates(
                testGen.tests.at(assertion_failures_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                    { [](tests::Tests::MethodTestCase const &testCase) {
                         return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 7;
                     },
                      [](tests::Tests::MethodTestCase const &testCase) {
                          return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 7;
                      } }),
                "buggy_function2");
        }

        static bool checkEquals(const tests::Tests::TestCaseParamValue& first, const tests::Tests::TestCaseParamValue& second){
            return std::stoi(first.view->getEntryValue(nullptr)) == std::stoi(second.view->getEntryValue(nullptr));
        }

        void checkDifferentVariables_C(BaseTestGen &testGen, bool differentVariables) {
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(different_variables_c).methods) {
                if (methodName == "swap_two_int_pointers") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [differentVariables](tests::Tests::MethodTestCase const &testCase) {
                                return (differentVariables ^
                                        checkEquals(testCase.paramValues[0],
                                                    testCase.paramValues[1])) &&
                                       checkEquals(testCase.paramPostValues[0],
                                                   testCase.paramValues[1]) &&
                                       checkEquals(testCase.paramPostValues[1],
                                                   testCase.paramValues[0]) &&
                                       testCase.stdinValue == std::nullopt;
                            } }),
                        methodName);
                } else if (methodName == "max_of_two_float") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [differentVariables](tests::Tests::MethodTestCase const &testCase) {
                                 return ((stod(testCase.paramValues[0].view->getEntryValue(
                                              nullptr)) ==
                                          stod(testCase.paramValues[1].view->getEntryValue(
                                              nullptr))) ^
                                         differentVariables) &&
                                        stod(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                            std::max(
                                                stod(testCase.paramValues[0].view->getEntryValue(
                                                    nullptr)),
                                                stod(testCase.paramValues[1].view->getEntryValue(
                                                    nullptr))) &&
                                        testCase.stdinValue == std::nullopt;
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return stod(testCase.paramValues[0].view->getEntryValue(
                                             nullptr)) !=
                                             stod(testCase.paramValues[1].view->getEntryValue(
                                                 nullptr)) &&
                                         stod(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                             std::max(
                                                 stod(testCase.paramValues[0].view->getEntryValue(
                                                     nullptr)),
                                                 stod(testCase.paramValues[1].view->getEntryValue(
                                                     nullptr))) &&
                                         testCase.stdinValue == std::nullopt;
                              } }),
                        methodName);
                } else if (methodName == "struct_test") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 return testCase.returnValue.view->getEntryValue(nullptr) == "-1";
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testCase.returnValue.view->getEntryValue(nullptr) == "1";
                              } }),
                        methodName);
                }
            }
        }

        void checkBasicFunctions_C(BaseTestGen &testGen) {
            EXPECT_EQ(printer::TestsPrinter::needsMathHeader(testGen.tests.at(basic_functions_c)),
              false);
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(basic_functions_c).methods) {
                if (methodName == "max_") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >
                                            stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                        stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                            stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) &&
                                        testCase.stdinValue == std::nullopt;
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) <=
                                             stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                         stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                             stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                         testCase.stdinValue == std::nullopt;
                              } }),
                        methodName);
                } else if (methodName == "sqr_positive") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0 &&
                                        stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1 &&
                                        testCase.stdinValue == std::nullopt;
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >= 0 &&
                                         stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                             stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) *
                                                 stoi(testCase.paramValues[0]
                                                          .view->getEntryValue(nullptr)) &&
                                         testCase.stdinValue == std::nullopt;
                              } }),
                        methodName);
                } else if (methodName == "simple_loop") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 return testCase.returnValue.view->getEntryValue(nullptr) == "0" &&
                                        testCase.stdinValue == std::nullopt;
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testCase.returnValue.view->getEntryValue(nullptr) == "1" &&
                                         testCase.stdinValue == std::nullopt;
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testCase.returnValue.view->getEntryValue(nullptr) == "2" &&
                                         testCase.stdinValue == std::nullopt;
                              } }),
                        methodName);
                }
            }
        }

        void checkDependentFunctions_C(BaseTestGen &testGen) {
            checkTestCasePredicates(
                testGen.tests.at(dependent_functions_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                    { [](tests::Tests::MethodTestCase const &testCase) {
                         return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                    stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) * 2;
                     },
                      [](tests::Tests::MethodTestCase const &testCase) {
                          return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) <=
                                     stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                 stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                     stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) * 2;
                      } }),
                "double_max");
        }

        void checkSimpleStructs_C(BaseTestGen &testGen) {
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(simple_structs_c).methods) {
                if (methodName == "get_sign_struct") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>({
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "0";
                            },
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "-1";
                            },
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "1";
                            },
                        }),
                        methodName);
                } else if (methodName == "calculate_something") {
                    testUtils::checkMinNumberOfTests(methodDescription.testCases, 5);
                } else if (methodName == "get_symbol_by_struct") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 return testUtils::cmpChars(
                                     testCase.returnValue.view->getEntryValue(nullptr), 'a');
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'c');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'u');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), '1');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), '0');
                              } }),
                        methodName);
                } else if (methodName == "operate_with_inner_structs") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 return testUtils::cmpChars(
                                     testCase.returnValue.view->getEntryValue(nullptr), '5');
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'e');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'g');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'o');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) { return true; } }),
                        methodName);
                } else if (methodName == "struct_as_return_type") {
                    testUtils::checkMinNumberOfTests(methodDescription.testCases, 3);
                }
            }
        }

        void checkSimpleUnions_C(BaseTestGen &testGen) {
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(simple_unions_c).methods) {
                if (methodName == "get_sign_union") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>({
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "0";
                            },
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "-1";
                            },
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "1";
                            },
                        }),
                        methodName);
                }
                if (methodName == "extract_bit") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>({
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "0";
                            },
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "-1";
                            },
                            [](tests::Tests::MethodTestCase const &testCase) {
                                return testCase.returnValue.view->getEntryValue(nullptr) == "1";
                            },
                        }),
                        methodName);
                }
                if (methodName == "calculate_something_union") {
                    testUtils::checkMinNumberOfTests(methodDescription.testCases, 5);
                }
                if (methodName == "get_coordinate") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                int i = stoi(testCase.paramValues[1].view->getEntryValue(nullptr));
                                return i >= 0 && i < 2;
                            } }),
                        methodName);
                }
                if (methodName == "operate_with_inner_unions") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 return testUtils::cmpChars(
                                     testCase.returnValue.view->getEntryValue(nullptr), '5');
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), '5');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), '5');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'e');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'f');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'g');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), 'o');
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testUtils::cmpChars(
                                      testCase.returnValue.view->getEntryValue(nullptr), '\x0f');
                              } }),
                        methodName);
                }
                if (methodName == "union_as_return_type") {
                    testUtils::checkMinNumberOfTests(methodDescription.testCases, 3);
                }
                if (methodName == "to_int") {
                    testUtils::checkMinNumberOfTests(methodDescription.testCases, 2);
                }
            }
        }

        void checkInnerBasicFunctions_C(BaseTestGen &testGen) {
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(inner_basic_functions_c).methods) {
                if (methodName == "sum_up_to") {
                    EXPECT_FALSE(methodDescription.testCases.empty());
                } else if (methodName == "median") {
                    checkTestCasePredicates(
                        methodDescription.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 return testCase.returnValue.view->getEntryValue(nullptr) ==
                                        testCase.paramValues[0].view->getEntryValue(nullptr);
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testCase.returnValue.view->getEntryValue(nullptr) ==
                                         testCase.paramValues[2].view->getEntryValue(nullptr);
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  return testCase.returnValue.view->getEntryValue(nullptr) ==
                                         testCase.paramValues[1].view->getEntryValue(nullptr);
                              } }),
                        methodName);
                }
            }
        }

        void checkPointerParameters_C(BaseTestGen &testGen) {
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(pointer_parameters_c).methods) {
                if (methodName == "c_strcmp") {
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                    for(const auto& testCase : methodDescription.testCases) {
                        EXPECT_TRUE(testCase.paramPostValues.empty());
                    }
                } else if (methodName == "ishello") {
                    EXPECT_FALSE(methodDescription.testCases.empty());
                    for(const auto& testCase : methodDescription.testCases) {
                        EXPECT_TRUE(testCase.paramPostValues.empty());
                    }
                } else if (methodName == "longptr_cmp") {
                    EXPECT_FALSE(methodDescription.testCases.empty());
                    for(const auto& testCase : methodDescription.testCases) {
                        EXPECT_EQ(testCase.paramPostValues.size(), 2);
                    }
                } else if (methodName == "accept_const_void_ptr_ptr") {
                    EXPECT_EQ(methodDescription.testCases.size(), 1);
                    for(const auto& testCase : methodDescription.testCases) {
                        EXPECT_TRUE(testCase.paramPostValues.empty());
                    }
                }
            }
        }

        void checkTypes_C(BaseTestGen &testGen) {
            for (const auto &[methodName, methodDescription] : testGen.tests.at(types_c).methods) {
                if (methodName == "a_or_b") {
                    EXPECT_GE(methodDescription.testCases.size(), 4);
                } else if (methodName == "max_long") {
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                } else if (methodName == "longptr_cmp") {
                    EXPECT_GE(methodDescription.testCases.size(), 3);
                }
            }
        }

        void checkPointerReturn_C(BaseTestGen &testGen) {
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(pointer_return_c).methods) {
                if (methodName == "returns_pointer_with_min") {
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                } else if (methodName == "returns_pointer_with_max") {
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                } else if (methodName == "five_square_numbers") {
                    EXPECT_GE(methodDescription.testCases.size(), 1);
                } else if (methodName == "returns_struct_with_min_max") {
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                } else if (methodName == "return_array_like_void_ptr") {
                    EXPECT_GE(methodDescription.testCases.size(), 1);
                }
            }
        }
        void checkFloatingPoint_C(BaseTestGen &testGen) {
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(floating_point_c).methods) {
                std::unordered_set<std::string> completeness;
                for (const auto &testCase : methodDescription.testCases) {
                    completeness.insert(testCase.returnValue.view->getEntryValue(nullptr));
                }
                if (methodName == "get_double_sign") {
                    EXPECT_GE(methodDescription.testCases.size(), 3);
                    EXPECT_GE(completeness.size(), 3);
                } else if (methodName == "is_close") {
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                    EXPECT_GE(completeness.size(), 2);
                } else if (methodName == "long_double_arith") {
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                    EXPECT_GE(completeness.size(), 2);
                } else if (methodName == "array_max") {
                    EXPECT_GE(methodDescription.testCases.size(), 3);
                    EXPECT_GE(completeness.size(), 3);
                } else if (methodName == "fp_array") {
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                    EXPECT_GE(completeness.size(), 2);
                }
            }
        }
        void checkFloatingPointPlain_C(BaseTestGen &testGen) {
            EXPECT_EQ(
            printer::TestsPrinter::needsMathHeader(testGen.tests.at(floating_point_plain_c)),
            true);
            for (const auto &[methodName, methodDescription] :
                 testGen.tests.at(floating_point_plain_c).methods) {
                if (methodName == "plain_isnan") {
                    std::unordered_set<std::string> completeness;
                    for (const auto &testCase : methodDescription.testCases) {
                        completeness.insert(testCase.returnValue.view->getEntryValue(nullptr));
                    }
                    EXPECT_GE(methodDescription.testCases.size(), 2);
                    EXPECT_GE(completeness.size(), 2);
                }
            }
        }

        void checkLinkage(BaseTestGen &testGen) {
            const auto &methods = testGen.tests.at(linkage_c).methods;
            EXPECT_EQ(methods.size(), 3);
            for (const auto &[methodName, methodDescription] : methods) {
                testUtils::checkMinNumberOfTests(methodDescription.testCases, 1);
            }
        }

        void checkGlobals(BaseTestGen &testGen) {
            auto const &methods = testGen.tests.at(globals_c).methods;
            EXPECT_EQ(methods.size(), 6);
            for (const auto &[_, md] : methods) {
                for (const auto &param : md.globalParams) {
                    EXPECT_NE(param.name, "externed_int_no_def");
                }
                if (md.name == "increment") {
                    checkTestCasePredicates(
                        md.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                auto returnValue = testCase.returnValue.view->getEntryValue(nullptr);
                                auto preValue = testCase.globalPreValues[0].view->getEntryValue(nullptr);
                                auto postValue = testCase.globalPostValues[0].view->getEntryValue(nullptr);
                                return returnValue == postValue &&
                                       stoi(preValue) + 1 == stoi(postValue);
                            } }));
                } else if (md.name == "use_globals") {
                    EXPECT_GE(md.testCases.size(), 3);
                    EXPECT_EQ(md.globalParams.size(), 3);
                } else if (md.name == "use_global_array") {
                    EXPECT_GE(md.testCases.size(), 3);
                    EXPECT_EQ(md.globalParams.size(), 1);
                } else if (md.name == "use_global_strings") {
                    EXPECT_GE(md.testCases.size(), 2);
                    EXPECT_EQ(md.globalParams.size(), 2);
                } else if (md.name == "use_global_arrays") {
                    EXPECT_GE(md.testCases.size(), 2);
                    EXPECT_EQ(md.globalParams.size(), 2);
                } else if (md.name == "use_global_handler") {
                    EXPECT_GE(md.testCases.size(), 1);
                    EXPECT_EQ(md.globalParams.size(), 0);
                }
            }
        }

        void checkKeywords(BaseTestGen &testGen) {
            auto const &methods = testGen.tests.at(keywords_c).methods;
            testUtils::checkMinNumberOfTests(testGen.tests, 13);
            for (const auto &[_, md] : methods) {
                if (md.name == "get_size_of_data") {
                    checkTestCasePredicates(md.testCases,
                                            std::vector<TestCasePredicate>(
                                                { [](tests::Tests::MethodTestCase const &testCase) {
                                                    auto returnValue =
                                                        testCase.returnValue.view->getEntryValue(nullptr);
                                                    return returnValue == "256";
                                                } }));
                } else if (md.name == "stop_now") {
                    checkTestCasePredicates(
                        md.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 auto i = testCase.paramValues[0].view->getEntryValue(nullptr);
                                 return i == "0";
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  auto i = testCase.paramValues[0].view->getEntryValue(nullptr);
                                  return stoi(i) > 0;
                              } }));
                } else if (md.name == "and") {
                    checkTestCasePredicates(
                        md.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                auto x = testCase.paramValues[0].view->getEntryValue(nullptr);
                                auto y = testCase.paramValues[1].view->getEntryValue(nullptr);
                                auto z = testCase.returnValue.view->getEntryValue(nullptr);
                                return (stoi(x) & stoi(y)) == stoi(z);
                            } }));
                } else if (md.name == "using") {
                    EXPECT_GE(md.testCases.size(), 1);
                } else if (md.name == "different") {
                    EXPECT_GE(md.testCases.size(), 1);
                    // should be 2 because aliasing is allowed there
                } else if (md.name == "not_null") {
                    EXPECT_GE(md.testCases.size(), 1);
                    // should be 2 because argument can be null
                } else if (md.name == "get_flag") {
                    EXPECT_GE(md.testCases.size(), 1);
                } else if (md.name == "cast") {
                    EXPECT_GE(md.testCases.size(), 1);
                } else if (md.name == "equals") {
                    EXPECT_GE(md.testCases.size(), 1);
                } else if (md.name == "access_to_int") {
                    checkTestCasePredicates(
                        md.testCases,
                        std::vector<TestCasePredicate>(
                            { [](tests::Tests::MethodTestCase const &testCase) {
                                 auto i = testCase.paramValues[0].view->getEntryValue(nullptr);
                                 return i == "private_";
                             },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  auto i = testCase.paramValues[0].view->getEntryValue(nullptr);
                                  return i == "protected_";
                              },
                              [](tests::Tests::MethodTestCase const &testCase) {
                                  auto i = testCase.paramValues[0].view->getEntryValue(nullptr);
                                  return i == "public_";
                              } }));
                }
            }
        }

        void checkAlignment(BaseTestGen &testGen) {
            auto const &methods = testGen.tests.at(alignment_c).methods;
            testUtils::checkMinNumberOfTests(testGen.tests, 1);
            for (const auto &[_, md] : methods) {
                if (md.name == "passthrough") {
                    checkTestCasePredicates(
                        md.testCases, std::vector<TestCasePredicate>(
                                          { [](tests::Tests::MethodTestCase const &testCase) {
                                              auto alignment = testCase.paramValues[0].alignment;
                                              return alignment == decltype(alignment){ 32768 };
                                          } }));
                }
            }
        }
    };

    TEST_F(Server_Test, Char_Literals_Test) {
        std::string suite = "char";
        setSuite(suite);
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 6);
        ASSERT_NO_THROW(testUtils::tryExecGetBuildCommands(getTestFilePath("tests")));
    }


    TEST_F(Server_Test, Linkage_Test) {
        std::string suite = "linkage";
        setSuite(suite);
        testUtils::tryExecGetBuildCommands(
            testUtils::getRelativeTestSuitePath(suite), CompilerName::CLANG,
            testUtils::BuildCommandsTool::BEAR_BUILD_COMMANDS_TOOL, true);

        fs::path a_c = getTestFilePath("a.c");
        fs::path b_c = getTestFilePath("b.c");
        fs::path main_c = getTestFilePath("main.c");
        {
            auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths, "ex");
            auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();

            checkTestCasePredicates(testGen.tests.at(b_c).methods.begin().value().testCases, {});

            checkTestCasePredicates(testGen.tests.at(a_c).methods.begin().value().testCases, {});

            checkTestCasePredicates(
                testGen.tests.at(main_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                    return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) - 2 ==
                           stoi(testCase.returnValue.view->getEntryValue(nullptr));
                } }));
        }
        {
            auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths, "one");
            auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();

            checkTestCasePredicates(
                testGen.tests.at(a_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                    return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) + 1 ==
                           stoi(testCase.returnValue.view->getEntryValue(nullptr));
                } }));
        }
        {
            auto request =
                createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths, "two");
            auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            ASSERT_TRUE(status.ok()) << status.error_message();

            checkTestCasePredicates(
                testGen.tests.at(b_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                    return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) - 1 ==
                           stoi(testCase.returnValue.view->getEntryValue(nullptr));
                } }));
        }

    }


    TEST_F(Server_Test, Datacom_Test) {
        std::string suite = "datacom";
        setSuite(suite);
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        auto const &methods = testGen.tests.at(getTestFilePath("main.c")).methods;
        EXPECT_EQ(methods.size(), 5);
        for (const auto &[_, md] : methods) {
            if (md.name == "main") {
                EXPECT_EQ(md.testCases.size(), 1);
            } else if (md.name == "FOO_FUNCTION_1") {
                auto inBounds = [](tests::Tests::MethodTestCase const &testCase) {
                    auto type = testCase.paramValues[0].view->getEntryValue(nullptr);
                    return stoi(type) >= 0 && stoi(type) < 3;
                };
                auto hasGlobalParameter = [](tests::Tests::MethodTestCase const &testCase) {
                    return testCase.globalPreValues.size() == 1;
                };
                checkTestCasePredicates(
                    md.testCases, std::vector<TestCasePredicate>({ inBounds, std::not_fn(inBounds) }));
                checkTestCasePredicates(md.testCases,
                                        std::vector<TestCasePredicate>{ 2, hasGlobalParameter });
            } else if (md.name == "FOO_FUNCTION_2") {
                EXPECT_GE(md.testCases.size(), 3);
            } else if (md.name == "FOO_FUNCTION_3") {
                EXPECT_GE(md.testCases.size(), 1);
            } else if (md.name == "FOO_FUNCTION_4") {
                EXPECT_GE(md.testCases.size(), 4);
            }
        }
    }

    TEST_F(Server_Test, Assertions_Failures) {
        auto [testGen, status] = performFeatureFileTestsRequest(assertion_failures_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkAssertionFailures_C(testGen);
    }

    TEST_F(Server_Test, Different_Variables_False) {
        auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                 GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, true, 60, ErrorMode::PASSING, false);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), different_variables_c);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        checkDifferentVariables_C(testGen, false);
    }

    TEST_F(Server_Test, Different_Variables_True) {
        auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                 GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, true, 60, ErrorMode::PASSING, true);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), different_variables_c);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        checkDifferentVariables_C(testGen, true);
    }

    TEST_F(Server_Test, Dependent_Functions) {
        auto [testGen, status] = performFeatureFileTestsRequest(dependent_functions_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkDependentFunctions_C(testGen);
    }

    TEST_F(Server_Test, Simple_Structs) {
        auto [testGen, status] = performFeatureFileTestsRequest(simple_structs_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkSimpleStructs_C(testGen);
    }

    TEST_F(Server_Test, Simple_Unions) {
        auto [testGen, status] = performFeatureFileTestsRequest(simple_unions_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkSimpleUnions_C(testGen);
    }

    TEST_F(Server_Test, Pointer_Parameters) {
        auto [testGen, status] = performFeatureFileTestsRequest(pointer_parameters_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkPointerParameters_C(testGen);
    }

    TEST_F(Server_Test, Types) {
        auto [testGen, status] = performFeatureFileTestsRequest(types_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkTypes_C(testGen);
    }

    TEST_F(Server_Test, Pointer_Return) {
        auto [testGen, status] = performFeatureFileTestsRequest(pointer_return_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkPointerReturn_C(testGen);
    }

    TEST_F(Server_Test, Floating_Point) {
        auto [testGen, status] = performFeatureFileTestsRequest(floating_point_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkFloatingPoint_C(testGen);
    }

    TEST_F(Server_Test, Floating_Point_plain) {
        auto [testGen, status] = performFeatureFileTestsRequest(floating_point_plain_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkFloatingPointPlain_C(testGen);
    }

    TEST_F(Server_Test, Correct_CodeText_For_Regression) {
        auto [testGen, status] = performFeatureFileTestsRequest(floating_point_plain_c);
        const std::string code = testGen.tests.begin()->second.code;
        const std::string beginRegressionRegion = "#pragma region " + Tests::DEFAULT_SUITE_NAME + NL;
        const std::string endRegion = std::string("#pragma endregion") + NL;
        const std::string beginErrorRegion = "#pragma region " + Tests::ERROR_SUITE_NAME + NL;
        ASSERT_TRUE(code.find(beginRegressionRegion) != std::string::npos) << "No regression begin region";
        ASSERT_TRUE(code.find(endRegion) != std::string::npos) << "No regression end region";
    }

    TEST_F(Server_Test, Linkage) {
        auto [testGen, status] = performFeatureFileTestsRequest(linkage_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkLinkage(testGen);
    }

    TEST_F(Server_Test, Globals) {
        auto [testGen, status] = performFeatureFileTestsRequest(globals_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkGlobals(testGen);
    }

    TEST_F(Server_Test, Keywords) {
        auto [testGen, status] = performFeatureFileTestsRequest(keywords_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkKeywords(testGen);
    }


    TEST_F(Server_Test, Alignment) {
        auto [testGen, status] = performFeatureFileTestsRequest(alignment_c);
        ASSERT_TRUE(status.ok()) << status.error_message();
        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";
        checkAlignment(testGen);
    }

    class Parameterized_Server_Test : public Server_Test,
                                      public testing::WithParamInterface<std::tuple<CompilerName>> {
    protected:
        void SetUp() override {
            clearEnv(std::get<0>(GetParam()));
            setCompiler(std::get<0>(GetParam()));
        }
    };

    const std::vector<CompilerName> compilerNames = { CompilerName::CLANG, CompilerName::GCC };

#define INSTANTIATE_TEST_SUITE_P_DifferentCompilers(test_suite_name)                               \
    INSTANTIATE_TEST_SUITE_P(                                                                      \
        DifferentCompilers, test_suite_name,                                                       \
        ::testing::Combine(::testing::ValuesIn(compilerNames)),                                    \
        [](const testing::TestParamInfo<Parameterized_Server_Test::ParamType> &info) {             \
            return to_string(std::get<0>(info.param));                                             \
        });

    INSTANTIATE_TEST_SUITE_P_DifferentCompilers(Parameterized_Server_Test);

    TEST_P(Parameterized_Server_Test, Snippet_Test) {
        auto request = createSnippetRequest(projectName, suitePath, snippet_c);
        auto testGen = SnippetTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_EQ(1, testFilePaths.size());
        EXPECT_EQ(1, testGen.tests.at(testFilePaths[0]).methods.size());
        EXPECT_EQ(1, testGen.tests.at(testFilePaths[0]).methods.begin().value().testCases.size());
        EXPECT_EQ(2, testGen.tests.at(testFilePaths[0])
                         .methods.begin()
                         .value()
                         .testCases[0]
                         .paramValues.size());

        fs::path stubPath = Paths::sourcePathToStubPath(testGen.projectContext, snippet_c);
        EXPECT_FALSE(fs::exists(stubPath)) << "Stub must not be generated: " << stubPath;
    }

    TEST_P(Parameterized_Server_Test, Project_Test) {
        std::string suite = "small-project";
        setSuite(suite);
        srcPaths = {suitePath, suitePath / "lib", suitePath / "src"};
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";

        for (const auto &test : testGen.tests) {
            for (const auto &[methodName, methodDescription] : test.second.methods) {
                 testUtils::checkMinNumberOfTests(methodDescription.testCases, 2);
            }
        }
    }

    TEST_P(Parameterized_Server_Test, Project_Test_Auto_Detect_Src_Paths) {
        std::string suite = "small-project";
        setSuite(suite);
        srcPaths = {};
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";

        EXPECT_FALSE(testFilePaths.size() < 2) << "Not enough test files are generated";
        EXPECT_FALSE(testFilePaths.size() > 2) << "More than needed test files are generated";

        for (const auto &test : testGen.tests) {
            for (const auto &[methodName, methodDescription] : test.second.methods) {
                testUtils::checkMinNumberOfTests(methodDescription.testCases, 2);
            }
        }
    }

    TEST_P(Parameterized_Server_Test, Project_Test_Detect_Src_Paths_From_Request) {
        std::string suite = "small-project";
        setSuite(suite);
        srcPaths = { suitePath / "lib"};
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        auto testFilePaths = CollectionUtils::getKeys(testGen.tests);
        EXPECT_TRUE(!testFilePaths.empty()) << "Generated test files are missing.";

        EXPECT_FALSE(testFilePaths.size() < 1) << "Not enough test files are generated";
        EXPECT_FALSE(testFilePaths.size() > 1) << "More than needed test files are generated";

        for (const auto &test : testGen.tests) {
            for (const auto &[methodName, methodDescription] : test.second.methods) {
                testUtils::checkMinNumberOfTests(methodDescription.testCases, 2);
            }
        }
    }

    TEST_P(Parameterized_Server_Test, File_Test) {
        auto projectRequest =
            createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), basic_functions_c);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 1);
        checkBasicFunctions_C(testGen);
    }

    TEST_P(Parameterized_Server_Test, Folder_Test) {
        auto projectRequest = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                                   GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto request = GrpcUtils::createFolderRequest(std::move(projectRequest), suitePath / "inner");
        auto testGen = FolderTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 1);
        checkInnerBasicFunctions_C(testGen);
    }

    TEST_P(Parameterized_Server_Test, Line_Test1) {
        auto request = createLineRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                         basic_functions_c, 17, GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 0);
        auto testGen = LineTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(basic_functions_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0 &&
                       stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                } }),
            "sqr_positive");
    }

    TEST_P(Parameterized_Server_Test, Line_Test2) {
        auto request = createLineRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                         basic_functions_c, 17, GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 0);
        auto testGen = LineTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(basic_functions_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                       stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) *
                           stoi(testCase.paramValues[0].view->getEntryValue(nullptr));
            } }),
            "sqr_positive");
    }

    TEST_P(Parameterized_Server_Test, Class_test1) {
        auto request = createClassRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                          multiple_classes_h, 6);
        auto testGen = ClassTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(multiple_classes_cpp).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                    return testCase.returnValue.view->getEntryValue(nullptr) == "1";} }),
                "get1");
    }

    TEST_P(Parameterized_Server_Test, Class_test2) {
        auto request = createClassRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                          multiple_classes_h, 11);
        auto testGen = ClassTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(multiple_classes_cpp).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                    return testCase.returnValue.view->getEntryValue(nullptr) == "2";} }),
                "get2");
    }

    TEST_P(Parameterized_Server_Test, DISABLED_Class_test3) {
        auto request = createClassRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                          multiple_classes_h, 14);
        auto testGen = ClassTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(multiple_classes_cpp).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                    return testCase.returnValue.view->getEntryValue(nullptr) == "2";} }),
                "get3");
    }

    TEST_P(Parameterized_Server_Test, Function_Test) {
        auto lineRequest = createLineRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                             basic_functions_c, 6, GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 0);
        auto request = GrpcUtils::createFunctionRequest(std::move(lineRequest));
        auto testGen = FunctionTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(basic_functions_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](tests::Tests::MethodTestCase const &testCase) {
                     return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >
                                stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                            stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                stoi(testCase.paramValues[0].view->getEntryValue(nullptr));
                 },
                  [](tests::Tests::MethodTestCase const &testCase) {
                      return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) <=
                                 stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                             stoi(testCase.returnValue.view->getEntryValue(nullptr)) ==
                                 stoi(testCase.paramValues[1].view->getEntryValue(nullptr));
                  } }),
            "max_");
    }

    TEST_P(Parameterized_Server_Test, Predicate_Test_Integer) {
        auto lineRequest = createLineRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                             basic_functions_c, 17, GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 0);
        auto predicateInfo = std::make_unique<testsgen::PredicateInfo>();
        predicateInfo->set_predicate("==");
        predicateInfo->set_returnvalue("36");
        predicateInfo->set_type(testsgen::INT32_T);
        auto request =
            GrpcUtils::createPredicateRequest(std::move(lineRequest), std::move(predicateInfo));
        auto testGen = PredicateTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(basic_functions_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return testCase.paramValues[0].view->getEntryValue(nullptr) == "6";
            } }),
            "sqr_positive");
    }

    TEST_P(Parameterized_Server_Test, Predicate_Test_Str) {
        auto lineRequest = createLineRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                             basic_functions_c, 32, GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 0);
        auto predicateInfo = std::make_unique<testsgen::PredicateInfo>();
        predicateInfo->set_predicate("==");
        predicateInfo->set_returnvalue("abacaba");
        predicateInfo->set_type(testsgen::STRING);
        auto request =
            GrpcUtils::createPredicateRequest(std::move(lineRequest), std::move(predicateInfo));
        auto testGen = PredicateTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(basic_functions_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return testCase.returnValue.view->getEntryValue(nullptr) == "\"abacaba\"";
            } }),
            "const_str");
    }

    TEST_P(Parameterized_Server_Test, Symbolic_Stdin_Test) {
        auto request = std::make_unique<FunctionRequest>();
        auto lineRequest = createLineRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                             symbolic_stdin_c, 8, GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 0);
        request->set_allocated_linerequest(lineRequest.release());
        auto testGen = FunctionTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        bool foundPath = false;
        for (const auto &testCase :
            testGen.tests.at(symbolic_stdin_c).methods.begin().value().testCases) {
            foundPath |= (testCase.returnValue.view->getEntryValue(nullptr) == "1");
            if (foundPath) {
                break;
            }
        }
        ASSERT_TRUE(foundPath);
    }

    TEST_P(Parameterized_Server_Test, Symbolic_Stdin_Long_Read) {
        auto request = std::make_unique<FunctionRequest>();
        auto lineRequest = createLineRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                             symbolic_stdin_c, 19, GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 0);
        request->set_allocated_linerequest(lineRequest.release());
        auto testGen = FunctionTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        ASSERT_EQ(1, testUtils::getNumberOfTests(testGen.tests));
        const auto testCases = testGen.tests.at(symbolic_stdin_c).methods.begin().value().testCases;
        ASSERT_FALSE(testCases.empty());
        ASSERT_FALSE(testCases[0].isError());
    }


    const std::string pregeneratedTestsRelativeDir = "pregenerated_tests";

    class TestRunner_Test : public Parameterized_Server_Test {
    protected:
        fs::path testDirPath;
        std::unique_ptr<utbot::ProjectContext> projectContext;

        fs::path basic_functions_c;
        fs::path simple_loop_uncovered_c;
        fs::path dependent_functions_c;
        fs::path simple_class_cpp;
        fs::path methods_with_exceptions_cpp;
        fs::path methods_with_asserts_cpp;

        fs::path dependent_functions_test_cpp;


        void SetUp() override {
            Parameterized_Server_Test::SetUp();
            setSuite("coverage");

            testDirPath = getTestFilePath(pregeneratedTestsRelativeDir);
            projectContext = std::make_unique<utbot::ProjectContext>(
                projectName, suitePath, testDirPath, buildDirRelativePath, clientProjectPath);

            basic_functions_c = getTestFilePath("basic_functions.c");
            simple_loop_uncovered_c = getTestFilePath("simple_loop_uncovered.c");
            dependent_functions_c = getTestFilePath("dependent_functions.c");
            simple_class_cpp = getTestFilePath("simple_class.cpp");
            methods_with_exceptions_cpp = getTestFilePath("methods_with_exceptions.cpp");
            methods_with_asserts_cpp = getTestFilePath("methods_with_asserts.cpp");

            dependent_functions_test_cpp =
                Paths::sourcePathToTestPath(*projectContext, dependent_functions_c);

            generateFiles(basic_functions_c, pregeneratedTestsRelativeDir);
            generateFiles(simple_loop_uncovered_c, pregeneratedTestsRelativeDir);
            generateFiles(dependent_functions_c, pregeneratedTestsRelativeDir);
            generateFiles(simple_class_cpp, pregeneratedTestsRelativeDir);
        }

        CoverageAndResultsGenerator generate(std::unique_ptr<testsgen::TestFilter> testFilter,
                                             bool withCoverage, ErrorMode errorMode = ErrorMode::FAILING) {
            auto request = createCoverageAndResultsRequest(
                projectName, suitePath, testDirPath, buildDirRelativePath, std::move(testFilter));
            static auto coverageAndResultsWriter =
                std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
            CoverageAndResultsGenerator coverageGenerator{ request.get(), coverageAndResultsWriter.get() };
            utbot::SettingsContext settingsContext{ true, true, 30, 0, true, false, errorMode, false};
            coverageGenerator.generate(withCoverage, settingsContext);
            EXPECT_FALSE(coverageGenerator.hasExceptions());
            return coverageGenerator;
        }
    };

    INSTANTIATE_TEST_SUITE_P_DifferentCompilers(TestRunner_Test);

    TEST_P(TestRunner_Test, Coverage_Project_Test) {
        CoverageLines linesCovered;
        CoverageLines linesUncovered;
        CoverageLines linesNone;

        // These are not all the lines
        linesCovered[basic_functions_c] = {2, 3, 4, 7, 13, 16, 18};
        linesUncovered[basic_functions_c] = {};
        linesNone[basic_functions_c] = { 0, 1, 10, 19 };

        linesCovered[simple_loop_uncovered_c] = {2, 3, 4, 5, 6, 7, 12, 13};
        linesUncovered[simple_loop_uncovered_c] = { 9 };
        linesNone[simple_loop_uncovered_c] = { 0, 1 };

        linesCovered[simple_class_cpp] = {10, 14, 18, 22, 23, 55, 58, 61, 64};
        linesUncovered[simple_class_cpp] = {};

        linesCovered[dependent_functions_c] = { 2, 3 };
        linesNone[dependent_functions_c] = { 0, 1 };

        auto testFilter = GrpcUtils::createTestFilterForProject();
        CoverageAndResultsGenerator coverageGenerator = generate(std::move(testFilter), true);
        const auto &coverageMap = coverageGenerator.getCoverageMap();
        EXPECT_FALSE(coverageMap.empty());
        testUtils::checkCoverage(coverageMap, linesCovered, linesUncovered, linesNone);
        EXPECT_GE(coverageGenerator.getTotals()["lines"]["percent"], 90);
    }

    TEST_P(TestRunner_Test, Coverage_File_Test) {
        CoverageLines linesCovered;
        CoverageLines linesUncovered;
        CoverageLines linesNone;

        // These are not all the lines
        linesCovered[basic_functions_c] = {2, 3, 4, 7};
        linesUncovered[basic_functions_c] =  {11, 12, 13, 16, 20, 21, 22, 25};
        linesNone[basic_functions_c] = {0, 1, 10, 19};

        linesCovered[simple_loop_uncovered_c] = {};
        linesUncovered[simple_loop_uncovered_c] = {};
        linesNone[simple_loop_uncovered_c] = {};

        linesCovered[dependent_functions_c] = { 2, 3 };
        linesNone[dependent_functions_c] = { 0, 1 };

        auto testFilter = GrpcUtils::createTestFilterForFile(dependent_functions_test_cpp);
        CoverageAndResultsGenerator coverageGenerator = generate(std::move(testFilter), true);
        const auto &coverageMap = coverageGenerator.getCoverageMap();
        EXPECT_FALSE(coverageMap.empty());
        testUtils::checkCoverage(coverageMap, linesCovered, linesUncovered, linesNone);
    }

    TEST_P(TestRunner_Test, Coverage_Case_Test) {
        CoverageLines linesCovered;
        CoverageLines linesUncovered;
        CoverageLines linesNone;

        // These are not all the lines
        linesCovered[basic_functions_c] = { 2, 3, 4 };
        linesUncovered[basic_functions_c] = {7, 11, 12, 13, 16, 20, 21, 22, 25};
        linesNone[basic_functions_c] =  {0, 1, 10, 19};

        linesCovered[simple_loop_uncovered_c] = {};
        linesUncovered[simple_loop_uncovered_c] = {};
        linesNone[simple_loop_uncovered_c] = {};

        linesCovered[dependent_functions_c] = { 2, 3 };
        linesNone[dependent_functions_c] = { 0, 1 };

        auto testFilter = GrpcUtils::createTestFilterForTest(dependent_functions_test_cpp,
                                                             "regression", "double_max_test_2");
        CoverageAndResultsGenerator coverageGenerator = generate(std::move(testFilter), true);
        const auto &coverageMap = coverageGenerator.getCoverageMap();
        EXPECT_FALSE(coverageMap.empty());
        testUtils::checkCoverage(coverageMap, linesCovered, linesUncovered, linesNone);
    }

    TEST_P(TestRunner_Test, Status_Test) {
        auto testFilter = GrpcUtils::createTestFilterForProject();
        CoverageAndResultsGenerator coverageGenerator = generate(std::move(testFilter), false);

        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        ASSERT_FALSE(resultMap.empty());

        testUtils::checkStatuses(resultMap, tests);

        StatusCountMap expectedStatusCountMap{{testsgen::TEST_PASSED, 25}};
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap);
    }

    TEST_F(Server_Test, Halt_Test) {
        std::string suite = "halt";
        setSuite(suite);
        auto request = createSnippetRequest(projectName, suitePath, suitePath / "raise.c");
        auto testGen = SnippetTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
    }

    TEST_F(Server_Test, Sleep_Test) {
        std::string suite = "halt";
        setSuite(suite);
        auto request = createSnippetRequest(projectName, suitePath, suitePath / "sleep.c");
        auto testGen = SnippetTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        testUtils::checkMinNumberOfTests(testGen.tests, 1);
    }

    TEST_F(Server_Test, Asm_Test) {
        std::string suite = "halt";
        setSuite(suite);
        fs::path asm_c = getTestFilePath("asm.c");
        auto request = createSnippetRequest(projectName, suitePath, asm_c);
        auto testGen = SnippetTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 1);
    }

    TEST_F(Server_Test, Memory_Test) {
        std::string suite = "halt";
        setSuite(suite);
        static const std::string memory_c = getTestFilePath("memory.c");
        auto request = createSnippetRequest(projectName, suitePath, memory_c);
        auto testGen = SnippetTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        auto const &methods = testGen.tests.at(memory_c).methods;
        /*
                checkTestCasePredicates(
                    methods.at("out_of_bound_access_to_heap").testCases,
                    vector<TestCasePredicate>(
                        { [](tests::Tests::MethodTestCase const &testCase) {
                          return testCase.isError();
                        },
                          [](tests::Tests::MethodTestCase const &testCase) {
                            return !testCase.isError();
                          } }),
                    "out_of_bound_access_to_heap");
        */
        checkTestCasePredicates(
            methods.at("out_of_bound_access_to_stack").testCases,
            std::vector<TestCasePredicate>(
                { [](tests::Tests::MethodTestCase const &testCase) { return testCase.isError(); },
                  [](tests::Tests::MethodTestCase const &testCase) {
                      return !testCase.isError();
                  } }),
            "out_of_bound_access_to_stack");
        checkTestCasePredicates(
            methods.at("out_of_bound_access_to_globals").testCases,
            std::vector<TestCasePredicate>(
                { [](tests::Tests::MethodTestCase const &testCase) { return testCase.isError(); },
                  [](tests::Tests::MethodTestCase const &testCase) {
                      return !testCase.isError();
                  } }),
            "out_of_bound_access_to_globals");
        checkTestCasePredicates(
            methods.at("use_after_free").testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return testCase.isError();
            } }),
            "use_after_free");
        checkTestCasePredicates(
            methods.at("leak_stack").testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return testCase.isError();
            } }),
            "leak_stack");
        checkTestCasePredicates(
            methods.at("use_after_return").testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return testCase.isError();
            } }),
            "use_after_return");
        /*
                checkTestCasePredicates(
                    methods.at("use_after_scope").testCases,
                    vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                      return testCase.isError();
                    } }),
                    "use_after_scope");
        */
        checkTestCasePredicates(
            methods.at("double_free").testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return testCase.isError();
            } }),
            "double_free");
        checkTestCasePredicates(
            methods.at("invalid_free").testCases,
            std::vector<TestCasePredicate>({ [](tests::Tests::MethodTestCase const &testCase) {
                return testCase.isError();
            } }),
            "invalid_free");
    }

    TEST_F(Server_Test, Object_File_Exe_Test) {
        std::string suite = "object-file";
        setSuite(suite);
        static const std::string source2_c = getTestFilePath("source2.c");
        auto projectRequest = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                                   GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 30, ErrorMode::FAILING);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), source2_c);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 2);

        auto testFilter = GrpcUtils::createTestFilterForProject();
        auto runRequest = createCoverageAndResultsRequest(
            projectName, suitePath, suitePath / "tests",
            buildDirRelativePath, std::move(testFilter));
        auto coverageAndResultsWriter = std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(), coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{ true, true, 45, 0, true, false, ErrorMode::FAILING, false};
        coverageGenerator.generate(false, settingsContext);

        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        ASSERT_FALSE(resultMap.empty());
        EXPECT_GT(resultMap.getNumberOfTests(), 2);

        testUtils::checkStatuses(resultMap, tests);

        StatusCountMap expectedStatusCountMap{{testsgen::TEST_PASSED, 7}};
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap);
    }

    TEST_F(Server_Test, Assert_Fail) {
        std::string suite = "error";
        setSuite(suite);

        testUtils::tryExecGetBuildCommands(
                testUtils::getRelativeTestSuitePath(suite), CompilerName::CLANG,
                testUtils::BuildCommandsTool::BEAR_BUILD_COMMANDS_TOOL, true);

        static const std::string methods_with_asserts_cpp = getTestFilePath("methods_with_asserts.cpp");
        auto projectRequest = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                                   GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 30,
                                                   ErrorMode::FAILING);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), methods_with_asserts_cpp);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 11);

        auto projectContext = std::make_unique<utbot::ProjectContext>(projectName, suitePath, suitePath / "tests",
                                                                      buildDirRelativePath, clientProjectPath);
        auto testFilter = GrpcUtils::createTestFilterForFile(
                Paths::sourcePathToTestPath(*projectContext, methods_with_asserts_cpp));
        auto runRequest = createCoverageAndResultsRequest(
                projectName, suitePath, suitePath / "tests",
                buildDirRelativePath, std::move(testFilter));
        auto coverageAndResultsWriter = std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{runRequest.get(), coverageAndResultsWriter.get()};
        utbot::SettingsContext settingsContext{true, true, 30, 0, true, false, ErrorMode::FAILING, false};
        coverageGenerator.generate(false, settingsContext);

        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        ASSERT_FALSE(resultMap.empty());
        EXPECT_GE(resultMap.getNumberOfTests(), 11);

        testUtils::checkStatuses(resultMap, tests);

        StatusCountMap expectedStatusCountMap{{testsgen::TEST_PASSED, 6}, {testsgen::TEST_DEATH, 5}};
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap, false);
    }



    TEST_F(Server_Test, Assert_Pass) {
        std::string suite = "error";
        setSuite(suite);

        testUtils::tryExecGetBuildCommands(
                testUtils::getRelativeTestSuitePath(suite), CompilerName::CLANG,
                testUtils::BuildCommandsTool::BEAR_BUILD_COMMANDS_TOOL, true);

        static const std::string methods_with_asserts_cpp = getTestFilePath("methods_with_asserts.cpp");
        auto projectRequest = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                                   GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 30,
                                                   ErrorMode::PASSING);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), methods_with_asserts_cpp);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 11);

        auto projectContext = std::make_unique<utbot::ProjectContext>(projectName, suitePath, suitePath / "tests",
                                                                      buildDirRelativePath, clientProjectPath);
        auto testFilter = GrpcUtils::createTestFilterForFile(
                Paths::sourcePathToTestPath(*projectContext, methods_with_asserts_cpp));
        auto runRequest = createCoverageAndResultsRequest(
                projectName, suitePath, suitePath / "tests",
                buildDirRelativePath, std::move(testFilter));
        auto coverageAndResultsWriter = std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{runRequest.get(), coverageAndResultsWriter.get()};
        utbot::SettingsContext settingsContext{true, true, 30, 0, true, false, ErrorMode::PASSING, false};
        coverageGenerator.generate(false, settingsContext);

        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        ASSERT_FALSE(resultMap.empty());
        EXPECT_GE(resultMap.getNumberOfTests(), 11);

        testUtils::checkStatuses(resultMap, tests);

        StatusCountMap expectedStatusCountMap{{testsgen::TEST_PASSED, 11}};
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap, false);
    }

    TEST_F(Server_Test, Exception_Fail) {
        std::string suite = "error";
        setSuite(suite);

        testUtils::tryExecGetBuildCommands(
                testUtils::getRelativeTestSuitePath(suite), CompilerName::CLANG,
                testUtils::BuildCommandsTool::BEAR_BUILD_COMMANDS_TOOL, true);

        static const std::string methods_with_exceptions_cpp = getTestFilePath("methods_with_exceptions.cpp");
        auto projectRequest = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                                   GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 30,
                                                   ErrorMode::FAILING);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), methods_with_exceptions_cpp);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 8);

        auto projectContext = std::make_unique<utbot::ProjectContext>(projectName, suitePath, suitePath / "tests",
                                                                      buildDirRelativePath, clientProjectPath);
        auto testFilter = GrpcUtils::createTestFilterForFile(
                Paths::sourcePathToTestPath(*projectContext, methods_with_exceptions_cpp));
        auto runRequest = createCoverageAndResultsRequest(
                projectName, suitePath, suitePath / "tests",
                buildDirRelativePath, std::move(testFilter));
        auto coverageAndResultsWriter = std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{runRequest.get(), coverageAndResultsWriter.get()};
        utbot::SettingsContext settingsContext{true, true, 30, 0, true, false, ErrorMode::FAILING, false};
        coverageGenerator.generate(false, settingsContext);

        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        ASSERT_FALSE(resultMap.empty());
        EXPECT_GE(resultMap.getNumberOfTests(), 8);

        testUtils::checkStatuses(resultMap, tests);

        StatusCountMap expectedStatusCountMap{{testsgen::TEST_PASSED, 4}, {testsgen::TEST_FAILED, 4}};
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap, false);
    }

    TEST_F(Server_Test, DISABLED_Exception_Pass) {
        std::string suite = "error";
        setSuite(suite);

        testUtils::tryExecGetBuildCommands(
                testUtils::getRelativeTestSuitePath(suite), CompilerName::CLANG,
                testUtils::BuildCommandsTool::BEAR_BUILD_COMMANDS_TOOL, true);

        static const std::string methods_with_exceptions_cpp = getTestFilePath("methods_with_exceptions.cpp");
        auto projectRequest = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                                   GrpcUtils::UTBOT_AUTO_TARGET_PATH, false, false, 30,
                                                   ErrorMode::PASSING);
        auto request = GrpcUtils::createFileRequest(std::move(projectRequest), methods_with_exceptions_cpp);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 8);

        auto projectContext = std::make_unique<utbot::ProjectContext>(projectName, suitePath, suitePath / "tests",
                                                                      buildDirRelativePath, clientProjectPath);
        auto testFilter = GrpcUtils::createTestFilterForFile(
                Paths::sourcePathToTestPath(*projectContext, methods_with_exceptions_cpp));
        auto runRequest = createCoverageAndResultsRequest(
                projectName, suitePath, suitePath / "tests",
                buildDirRelativePath, std::move(testFilter));
        auto coverageAndResultsWriter = std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{runRequest.get(), coverageAndResultsWriter.get()};
        utbot::SettingsContext settingsContext{true, true, 30, 0, true, false, ErrorMode::PASSING, false};
        coverageGenerator.generate(false, settingsContext);

        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        ASSERT_FALSE(resultMap.empty());
        EXPECT_GE(resultMap.getNumberOfTests(), 8);

        testUtils::checkStatuses(resultMap, tests);

        StatusCountMap expectedStatusCountMap{{testsgen::TEST_PASSED, 8}};
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap, false);
    }

    struct ProjectInfo {
        fs::path projectRelativePath;
        int numberOfTests;
    };

    class Parameterized_Status_Server_Test
        : public Server_Test,
          public testing::WithParamInterface<std::tuple<CompilerName, ProjectInfo>> {
    protected:
        int numberOfTests{};
        int timeout;
        fs::path testsDirPath;

        void SetUp() override {
            clearEnv(std::get<0>(GetParam()));
            setCompiler(std::get<0>(GetParam()));
            setSuite("run");
            const auto &[subProjectName, numberOfTests] = std::get<1>(GetParam());
            suitePath /= subProjectName;
            buildPath = suitePath / buildDirRelativePath;
            this->numberOfTests = numberOfTests;
            this->testsDirPath = getTestFilePath(pregeneratedTestsRelativeDir);
            timeout = (subProjectName == "timeout") ? 5 : 0;
        }

        void TearDown() override {
        }
    };

    const std::vector<ProjectInfo> testRelativeDirs = { { fs::path("executable"), 3 },
                                                        { fs::path("static_library"), 3 },
                                                        { fs::path("shared_library"), 3 },
                                                        { fs::path("timeout"), 1 } };

    INSTANTIATE_TEST_SUITE_P(
        DifferentCompilersAndSuites,
        Parameterized_Status_Server_Test,
        ::testing::Combine(::testing::ValuesIn(compilerNames),
                           ::testing::ValuesIn(testRelativeDirs)),
        [](const testing::TestParamInfo<Parameterized_Status_Server_Test::ParamType> &info) {
            auto compilerName = std::get<0>(info.param);
            auto projectRelativePath = std::get<1>(info.param).projectRelativePath;
            auto subProjectName = projectRelativePath.begin()->c_str();
            return StringUtils::stringFormat("%s_%s", to_string(compilerName).c_str(),
                                             subProjectName);
        });

    TEST_P(Parameterized_Status_Server_Test, Run_Test) {
        generateMakefilesForProject(pregeneratedTestsRelativeDir);

        auto testFilter = GrpcUtils::createTestFilterForProject();
        auto request = createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath,
            buildDirRelativePath, std::move(testFilter));
        auto coverageAndResultsWriter = std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ request.get(), coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{ true, true, 15, timeout, true, false, ErrorMode::FAILING, false};
        coverageGenerator.generate(false, settingsContext);

        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        ASSERT_FALSE(resultMap.empty());
        EXPECT_EQ(this->numberOfTests, resultMap.getNumberOfTests());
        if (timeout) {
            EXPECT_EQ(testsgen::TestStatus::TEST_INTERRUPTED,
                      resultMap.begin()->second.begin()->second.status());
        } else {
            testUtils::checkStatuses(resultMap, tests);

            StatusCountMap expectedStatusCountMap{
                {testsgen::TEST_PASSED, 3}};
            testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap);
        }
    }

    TEST_P(Parameterized_Server_Test, Clang_Resources_Directory_Test) {
        std::string suite = "stddef";
        setSuite(suite);
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 2);
    }

    TEST_P(Parameterized_Server_Test, Installed_Dependency_Test) {
        std::string suite = "installed";
        setSuite(suite);
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 2);
        auto const& cases = testGen.tests.begin().value().methods["display_version"].testCases;
        ASSERT_EQ(1, cases.size());
        ASSERT_FALSE(cases[0].isError());
    }

    TEST_P(Parameterized_Server_Test, Stats_Test) {
        std::string suite = "small-project";
        setSuite(suite);
        srcPaths = {};
        auto request = createProjectRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                            GrpcUtils::UTBOT_AUTO_TARGET_PATH);
        auto testGen = ProjectTestGen(*request, writer.get(), TESTMODE);

        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Server_Test, Run_Tests_For_Linked_List) {
        fs::path linked_list_c = getTestFilePath("linked_list.c");
        auto request = testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath,
                                                    srcPaths, linked_list_c,
                                                    GrpcUtils::UTBOT_AUTO_TARGET_PATH, true, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_GE(testUtils::getNumberOfTests(testGen.tests), 2);

        fs::path testsDirPath = getTestFilePath("tests");

        fs::path linked_list_test_cpp = Paths::sourcePathToTestPath(
            utbot::ProjectContext(projectName, suitePath, testsDirPath, buildDirRelativePath, clientProjectPath),
            linked_list_c);
        auto testFilter = GrpcUtils::createTestFilterForFile(linked_list_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
            std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(),
                                                       coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{
            true, false, 45, 0, false, false, ErrorMode::FAILING, false
        };
        coverageGenerator.generate(false, settingsContext);

        EXPECT_FALSE(coverageGenerator.hasExceptions());
        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultsMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        // TODO: add checkStatusesCount after linkedlist fix
        testUtils::checkStatuses(resultsMap, tests);
    }

    TEST_F(Server_Test, Run_Tests_For_Tree) {
        fs::path tree_c = getTestFilePath("tree.c");
        auto request =
            testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                         tree_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_GE(testUtils::getNumberOfTests(testGen.tests), 2);

        fs::path testsDirPath = getTestFilePath("tests");

        fs::path tree_test_cpp = Paths::sourcePathToTestPath(
            utbot::ProjectContext(projectName, suitePath, testsDirPath, buildDirRelativePath, clientProjectPath),
            tree_c);
        auto testFilter = GrpcUtils::createTestFilterForFile(tree_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
            std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(),
                                                       coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{
            true, false, 15, 0, false, false, ErrorMode::FAILING, false
        };
        coverageGenerator.generate(false, settingsContext);

        EXPECT_FALSE(coverageGenerator.hasExceptions());
        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        testUtils::checkStatuses(resultMap, tests);

        StatusCountMap expectedStatusCountMap{ { testsgen::TEST_PASSED, 6 } };
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap);
    }

    TEST_F(Server_Test, Run_Tests_For_Input_Output_C) {
        fs::path input_output_c = getTestFilePath("input_output.c");
        auto request = testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath,
                                                    srcPaths, input_output_c,
                                                    GrpcUtils::UTBOT_AUTO_TARGET_PATH, true, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        testGen.setTargetForSource(input_output_c);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_EQ(testUtils::getNumberOfTests(testGen.tests),
                  63); // 61 regression tests and 2 error

        fs::path testsDirPath = getTestFilePath("tests");

        fs::path input_output_test_cpp = Paths::sourcePathToTestPath(
            utbot::ProjectContext(projectName, suitePath, testsDirPath, buildDirRelativePath, clientProjectPath),
            input_output_c);
        auto testFilter = GrpcUtils::createTestFilterForFile(input_output_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
            std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(),
                                                       coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{
            true, false, 45, 30, false, false, ErrorMode::FAILING, false
        };
        coverageGenerator.generate(false, settingsContext);

        EXPECT_FALSE(coverageGenerator.hasExceptions());
        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        StatusCountMap expectedStatusCountMap{ { testsgen::TEST_PASSED, 61 } };
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap);
    }

    TEST_F(Server_Test, Run_Tests_For_File_C) {
        fs::path file_c = getTestFilePath("file.c");
        auto request =
            testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath, srcPaths,
                                         file_c, GrpcUtils::UTBOT_AUTO_TARGET_PATH, true, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        testGen.setTargetForSource(file_c);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_EQ(testUtils::getNumberOfTests(testGen.tests), 33);

        fs::path testsDirPath = getTestFilePath("tests");

        fs::path file_test_cpp = Paths::sourcePathToTestPath(
            utbot::ProjectContext(projectName, suitePath, testsDirPath, buildDirRelativePath, clientProjectPath),
            file_c);
        auto testFilter = GrpcUtils::createTestFilterForFile(file_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
            std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(),
                                                       coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{
            true, false, 45, 0, false, false, ErrorMode::FAILING, false
        };
        coverageGenerator.generate(false, settingsContext);

        EXPECT_FALSE(coverageGenerator.hasExceptions());
        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        StatusCountMap expectedStatusCountMap{ { testsgen::TEST_PASSED, 33 } };
        testUtils::checkStatusesCount(resultMap, tests, expectedStatusCountMap);
    }

    TEST_F(Server_Test, Run_Tests_For_Hard_Linked_List) {
        fs::path hard_linked_list_c = getTestFilePath("hard_linked_list.c");
        auto request = testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath,
                                                    srcPaths, hard_linked_list_c,
                                                    GrpcUtils::UTBOT_AUTO_TARGET_PATH, true, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_GE(testUtils::getNumberOfTests(testGen.tests), 9);

        fs::path testsDirPath = getTestFilePath("tests");

        fs::path hard_linked_list_test_cpp = Paths::sourcePathToTestPath(
            utbot::ProjectContext(projectName, suitePath, testsDirPath, buildDirRelativePath, clientProjectPath),
            hard_linked_list_c);
        auto testFilter = GrpcUtils::createTestFilterForFile(hard_linked_list_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
            std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(),
                                                       coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{
            true, false, 45, 0, false, false, ErrorMode::FAILING, false
        };
        coverageGenerator.generate(false, settingsContext);

        EXPECT_FALSE(coverageGenerator.hasExceptions());
        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto resultsMap = coverageGenerator.getTestResultMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        StatusCountMap expectedStatusCountMap{ { testsgen::TEST_PASSED, 9 } };
        testUtils::checkStatuses(resultsMap, tests);
    }
}
