/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "gtest/gtest.h"

#include "BaseTest.h"
#include "KleeGenerator.h"
#include "Server.h"
#include "streams/coverage/ServerCoverageAndResultsWriter.h"
#include "coverage/CoverageAndResultsGenerator.h"

#include "utils/path/FileSystemPath.h"
#include <functional>

namespace {
    using grpc::Channel;
    using grpc::ClientContext;
    using testsgen::TestsGenService;
    using testsgen::TestsResponse;
    using testUtils::checkTestCasePredicates;
    using testUtils::createLineRequest;
    using CompilationUtils::CompilerName;

    class Syntax_Test : public BaseTest {
    protected:
        Syntax_Test() : BaseTest("syntax") {}

        fs::path simple_structs_c = getTestFilePath("simple_structs.c");
        fs::path simple_unions_c = getTestFilePath("simple_unions.c");
        fs::path pointer_return_c = getTestFilePath("pointer_return.c");
        fs::path pointer_parameters_c = getTestFilePath("pointer_parameters.c");
        fs::path complex_structs_c = getTestFilePath("complex_structs.c");
        fs::path types_c = getTestFilePath("types.c");
        fs::path types_3_c = getTestFilePath("types_3.c");
        fs::path typedefs_1_c = getTestFilePath("typedefs_1.c");
        fs::path typedefs_2_c = getTestFilePath("typedefs_2.c");
        fs::path enums_c = getTestFilePath("enums.c");
        fs::path constants_c = getTestFilePath("constants.c");
        fs::path packed_structs_c = getTestFilePath("packed_structs.c");
        fs::path void_functions_c = getTestFilePath("void_functions.c");
        fs::path qualifiers_c = getTestFilePath("qualifiers.c");
        fs::path structs_with_pointers_c = getTestFilePath("structs_with_pointers.c");
        fs::path struct_with_union_c = getTestFilePath("struct_with_union.c");
        fs::path functions_as_params_c = getTestFilePath("functions_as_params.c");
        fs::path multi_arrays_c = getTestFilePath("multi_arrays.c");
        fs::path variadic_c = getTestFilePath("variadic.c");
        fs::path floats_special_c = getTestFilePath("floats_special.c");
        fs::path linked_list_c = getTestFilePath("linked_list.c");
        fs::path tree_c = getTestFilePath("tree.c");
        fs::path different_parameters_cpp = getTestFilePath("different_parameters.cpp");
        fs::path simple_class_cpp = getTestFilePath("simple_class.cpp");
        fs::path inner_unnamed_c = getTestFilePath("inner_unnamed.c");
        fs::path array_sort_c = getTestFilePath("array_sort.c");
        fs::path stubs_c = getTestFilePath("stubs.c");
        fs::path namespace_cpp = getTestFilePath("namespace.cpp");

        void SetUp() override {
            clearEnv();
        }

        void checkReturnEnum(FunctionTestGen &testGen) {
            checkTestCasePredicates(
                    testGen.tests.at(enums_c).methods.begin().value().testCases,
                    std::vector<TestCasePredicate>(
                            {[] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0
                                    && testCase.returnValue.view->getEntryValue(nullptr) == "ZERO";
                            },
                             [] (const tests::Tests::MethodTestCase& testCase) {
                                 return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) > 0
                                    && testCase.returnValue.view->getEntryValue(nullptr) == "POSITIVE";
                             },
                             [] (const tests::Tests::MethodTestCase& testCase) {
                                 return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0
                                    && testCase.returnValue.view->getEntryValue(nullptr) == "NEGATIVE";
                             }
                            }
                    )
            );
        }

        std::pair<FunctionTestGen, Status> createTestForFunction(const fs::path &pathToFile,
                                                                 int lineNum, int kleeTimeout = 60) {
            auto lineRequest = createLineRequest(projectName, suitePath, buildDirRelativePath,
                                                 srcPaths, pathToFile, lineNum, false, kleeTimeout);
            auto request = GrpcUtils::createFunctionRequest(std::move(lineRequest));
            auto testGen = FunctionTestGen(*request, writer.get(), TESTMODE);
            testGen.setTargetForSource(pathToFile);
            Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
            size_t failed_build = TestRunner::buildTests(testGen.projectContext, testGen.tests);
            if (status.ok() && failed_build != 0) {
                std::string message = StringUtils::stringFormat("Build tests failed: %d", failed_build);
                LOG_S(ERROR) << message;
                return { testGen, Status(StatusCode::FAILED_PRECONDITION, message) };
            }
            return { testGen, status };
        }
    };

    TEST_F(Syntax_Test, Struct_Parameter_Test_1) {
        auto [testGen, status] = createTestForFunction(simple_structs_c, 9);

        printer::TestsPrinter testsPrinter(nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_structs_c)
                                .methods.begin().value().testCases.begin();
        ASSERT_EQ("{"
                  "\n    .x = 0,"
                  "\n    .a = 2}", tests[0].paramValues[0].view->getEntryValue(&testsPrinter));

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
                testGen.tests.at(simple_structs_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find(", 0}") != std::string::npos;
                            },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find(", -") != std::string::npos;
                            },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find(", -") == std::string::npos
                                    && testCase.paramValues[0].view->getEntryValue(nullptr).find(", 0}") == std::string::npos;
                         }
                        }),
                "get_sign_struct");
    }

    TEST_F(Syntax_Test, Struct_Parameter_Test_2) {
        auto [testGen, status] = createTestForFunction(simple_structs_c, 37);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(simple_structs_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {
                         [] (const tests::Tests::MethodTestCase& testCase) {
                            return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), 'a');
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), 'c');
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), 'u');
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '1');
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '0');
                         },
                        }),
                "get_symbol_by_struct");
    }

    TEST_F(Syntax_Test, Struct_Return_Test) {
        auto [testGen, status] = createTestForFunction(simple_structs_c, 78);

        ASSERT_TRUE(status.ok()) << status.error_message();

        printer::TestsPrinter testsPrinter(nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_structs_c)
                .methods.begin().value().testCases.begin();
        ASSERT_EQ("{"
                  "\n    .inner = {"
                  "\n        .c = '2',"
                  "\n        .ininner = {"
                  "\n            .u = 2U,"
                  "\n            .l = 2LL},"
                  "\n        .s = 2},"
                  "\n    .x = 2,"
                  "\n    .y = 2LL}", tests[0].returnValue.view->getEntryValue(&testsPrinter));

        checkTestCasePredicates(
                testGen.tests.at(simple_structs_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0
                               && testCase.returnValue.view->getEntryValue(nullptr) == "{{'0', {0U, 0LL}, 0}, 0, 0LL}";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 1
                               && testCase.returnValue.view->getEntryValue(nullptr) == "{{'1', {1U, 1LL}, 1}, 1, 1LL}";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 0
                               && stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 1
                                && testCase.returnValue.view->getEntryValue(nullptr) == "{{'2', {2U, 2LL}, 2}, 2, 2LL}";
                         },
                        }),
                "struct_as_return_type");
    }


    TEST_F(Syntax_Test, Union_Parameter_Test_1) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 9);

        ASSERT_TRUE(status.ok()) << status.error_message();


        checkTestCasePredicates(
                testGen.tests.at(simple_unions_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0 &&
                                testCase.paramValues[0].view->getEntryValue(nullptr) == "from_bytes<IntBytesUnion>({0, 0, 0, 0})";
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1 &&
                                testCase.paramValues[0].view->getEntryValue(nullptr) != "from_bytes<IntBytesUnion>({0, 0, 0, 0})";
                         },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1 &&
                                testCase.paramValues[0].view->getEntryValue(nullptr) != "from_bytes<IntBytesUnion>({0, 0, 0, 0})";
                         }
                        }),
                "get_sign_union");
    }

    TEST_F(Syntax_Test, Union_Parameter_Test_2) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 19);

        ASSERT_TRUE(status.ok()) << status.error_message();


        checkTestCasePredicates(
                testGen.tests.at(simple_unions_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find("from_bytes<ShortBytesUnion>({0, ") == 0;
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find("from_bytes<ShortBytesUnion>({0, ") == std::string::npos;
                         },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find("from_bytes<ShortBytesUnion>({0, 0}") == 0;
                         }
                        }),
                "extract_bit");
    }

    TEST_F(Syntax_Test, Union_Return_Test) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 78);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(simple_unions_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0
                               && testCase.returnValue.view->getEntryValue(nullptr) == "from_bytes<MainUnion>({48, 0, 0, 0, 0, 0, 0, 0})";
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 1
                               && testCase.returnValue.view->getEntryValue(nullptr) == "from_bytes<MainUnion>({1, 0, 0, 0, 0, 0, 0, 0})";
                         },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 0
                               && stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 1
                                && testCase.returnValue.view->getEntryValue(nullptr) == "from_bytes<MainUnion>({2, 0, 0, 0, 0, 0, 0, 0})";
                         },
                        }),
                "union_as_return_type");
    }

    TEST_F(Syntax_Test, Union_Array_Test) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 106);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(simple_unions_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            size_t it = 0;
                            int cnt = 0;
                            auto const &str = testCase.paramValues[0];
                            const char *substr = "}),";
                            while ((it = str.view->getEntryValue(nullptr).find(substr, it)) != std::string::npos) {
                                cnt++;
                                it++;
                            }
                            return cnt == 10 - 1;
                        }}),
                "sumOfUnionArray");
    }

    TEST_F(Syntax_Test, Union_With_Pointer_Test) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 116);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(simple_unions_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
            } }),
            "operateWithUnionWithPointer");
    }

    TEST_F(Syntax_Test, Pointer_Return_Test_1) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 12);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoll(testCase.paramValues[0].view->getEntryValue(nullptr)) < stoll(testCase.paramValues[1].view->getEntryValue(nullptr))
                                && stoll(testCase.paramValues[0].view->getEntryValue(nullptr)) == stoll(testCase.returnValue.view->getEntryValue(nullptr));
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoll(testCase.paramValues[0].view->getEntryValue(nullptr)) >= stoll(testCase.paramValues[1].view->getEntryValue(nullptr))
                                && stoll(testCase.paramValues[1].view->getEntryValue(nullptr)) == stoll(testCase.returnValue.view->getEntryValue(nullptr));
                         }
                        }),
                "returns_pointer_with_min");
    }

    TEST_F(Syntax_Test, Pointer_Return_Test_2) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 44);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoll(testCase.paramValues[0].view->getEntryValue(nullptr)) < stoll(testCase.paramValues[1].view->getEntryValue(nullptr))
                                && "{" + testCase.paramValues[0].view->getEntryValue(nullptr) + ", " + testCase.paramValues[1].view->getEntryValue(nullptr) + "}"
                                    == testCase.returnValue.view->getEntryValue(nullptr);
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoll(testCase.paramValues[0].view->getEntryValue(nullptr)) >= stoll(testCase.paramValues[1].view->getEntryValue(nullptr))
                                && "{" + testCase.paramValues[1].view->getEntryValue(nullptr) + ", " + testCase.paramValues[0].view->getEntryValue(nullptr) + "}"
                                    == testCase.returnValue.view->getEntryValue(nullptr);
                         }
                        }),
                "returns_struct_with_min_max");
    }

    TEST_F(Syntax_Test, Pointer_Return_Test_3) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 83);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                auto entryValue = testCase.paramValues[0].view->getEntryValue(nullptr);
                auto returnValue = stoll(testCase.returnValue.view->getEntryValue(nullptr));
                return static_cast<unsigned char>(entryValue[1]) ==
                       static_cast<unsigned char>(returnValue);
            }
                }),
            "void_pointer_return_char_usage");
    }

    TEST_F(Syntax_Test, Return_Long_Long_Array) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 94);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return /*stoll(testCase.paramValues[0].view->getEntryValue(nullptr)) == stoll(testCase.returnValue.view->getSubViews()[5]->getEntryValue(nullptr))
                         && */stoll(testCase.paramValues[1].view->getEntryValue(nullptr)) == stoll(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr));
                }
                }),
            "return_long_long_array");
    }

    TEST_F(Syntax_Test, Pointer_As_Array_Parameter) {
        auto [testGen, status] = createTestForFunction(pointer_parameters_c, 34);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_parameters_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.paramValues[2].view->getEntryValue(nullptr)) + 7
                        == stoi(testCase.returnValue.view->getEntryValue(nullptr))
                        && stoi(testCase.paramPostValues[0].view->getSubViews()[1]->getEntryValue(nullptr))
                        == 3
                        && stoi(testCase.paramPostValues[1].view->getEntryValue(nullptr))
                        == 4;
                }
                }),
            "pointer_as_array_parameter");
    }

    TEST_F(Syntax_Test, Structs_With_Arrays_Parameter_Test_1) {
        auto [testGen, status] = createTestForFunction(complex_structs_c, 11);

        ASSERT_TRUE(status.ok()) << status.error_message();

        const std::string alphabet = "{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L'}";
        checkTestCasePredicates(
                testGen.tests.at(complex_structs_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[&alphabet] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1 &&
                                testCase.paramValues[0].view->getEntryValue(nullptr).find(alphabet) != std::string::npos;
                        },
                         [&alphabet] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0 &&
                                 testCase.paramValues[0].view->getEntryValue(nullptr).find(alphabet) == std::string::npos;
                         }
                        }),
                "struct_has_alphabet");
    }

    TEST_F(Syntax_Test, Structs_With_Arrays_Return_Test_1) {
        auto [testGen, status] = createTestForFunction(complex_structs_c, 43);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(complex_structs_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoll(testCase.paramValues[0].view->getEntryValue(nullptr)) >= 0 &&
                                "{1, {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L'}}" == testCase.returnValue.view->getEntryValue(nullptr);
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                            return stoll(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0 &&
                                "{-1, {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'}}" == testCase.returnValue.view->getEntryValue(nullptr);
                         }
                        }),
                "alphabet");
    }

    TEST_F(Syntax_Test, Struct_With_Double_Pointer) {
        auto [testGen, status] = createTestForFunction(complex_structs_c, 58);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(complex_structs_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                    std::string expectedString = StringUtils::stringFormat("{%s, {%s, %s}, 0}",
                                                                      PrinterUtils::C_NULL, PrinterUtils::C_NULL, PrinterUtils::C_NULL);
                  return testCase.returnValue.view->getEntryValue(nullptr) ==  expectedString &&
                        testCase.paramValues[0].view->getEntryValue(nullptr) == expectedString &&
                        testCase.globalPostValues[0].view->getEntryValue(nullptr) == expectedString;
                }
                }),
            "check_double_pointer");
    }

    TEST_F(Syntax_Test, Booleans_as_Parameters_Test) {
        auto [testGen, status] = createTestForFunction(types_c, 50);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(types_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "false"
                                && testCase.paramValues[1].view->getEntryValue(nullptr) == "false" && testCase.returnValue.view->getEntryValue(nullptr) == "4";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "false"
                                && testCase.paramValues[1].view->getEntryValue(nullptr) == "true" && testCase.returnValue.view->getEntryValue(nullptr) == "3";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "true"
                                && testCase.paramValues[1].view->getEntryValue(nullptr) == "false" && testCase.returnValue.view->getEntryValue(nullptr) == "2";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "true"
                                && testCase.paramValues[1].view->getEntryValue(nullptr) == "true" && testCase.returnValue.view->getEntryValue(nullptr) == "1";
                        }
                        }),
                "fun_that_accept_bools");
    }

    TEST_F(Syntax_Test, Boolean_as_Return_Test) {
        auto [testGen, status] = createTestForFunction(types_c, 56);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(types_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) > 0 && testCase.returnValue.view->getEntryValue(nullptr) == "true";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) <= 0 && testCase.returnValue.view->getEntryValue(nullptr) == "false";
                         }
                        }),
                "is_positive");
    }

    TEST_F(Syntax_Test, Enum_as_Parameter_Test) {
        auto [testGen, status] = createTestForFunction(enums_c, 11);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(enums_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "NEGATIVE" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "ZERO" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "POSITIVE" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                         }
                        }
                )
        );
    }

    TEST_F(Syntax_Test, Void_Pointer_as_Parameter_Test) {
        auto [testGen, status] = createTestForFunction(pointer_parameters_c, 28);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_parameters_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return testCase.paramValues[0].view->getEntryValue(nullptr) == "0" && stoll(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                }
                }),
            "void_pointer_int_usage");
    }

    TEST_F(Syntax_Test, Enum_Pointer_as_Parameter_Test) {
        auto [testGen, status] = createTestForFunction(enums_c, 43);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(enums_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr).find("NEGATIVE") != std::string::npos
                                && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                           return testCase.paramValues[0].view->getEntryValue(nullptr).find("POSITIVE") != std::string::npos
                                && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr).find("ZERO") != std::string::npos
                                && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                         }
                        }
                )
        );
    }

    TEST_F(Syntax_Test, Enum_as_Return_Test) {
        auto [testGen, status] = createTestForFunction(enums_c, 22);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkReturnEnum(testGen);
    }

    TEST_F(Syntax_Test, Enum_Pointer_as_Return_Test) {
        auto [testGen, status] = createTestForFunction(enums_c, 47);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkReturnEnum(testGen);
    }


    TEST_F(Syntax_Test, Enum_in_Struct_Test) {
        auto [testGen, status] = createTestForFunction(enums_c, 30);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(enums_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {

                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "{ZERO}" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "{POSITIVE}" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) > 0;
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "{NEGATIVE}" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) < 0;
                         }
                        }
                )
        );
    }

    TEST_F(Syntax_Test, Enum_Out_Of_Bound_Value) {
        auto [testGen, status] = createTestForFunction(enums_c, 55);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(enums_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "NEGATIVE" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "ZERO" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                         },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "POSITIVE" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                         }
                        }
                ),
                "getSignValue"
        );
    }

    TEST_F(Syntax_Test, Enum_Withing_Record) {
        auto [testGen, status] = createTestForFunction(enums_c, 73);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(enums_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                     return testCase.paramValues[0].view->getEntryValue(nullptr) ==
                                "{EnumWithinRecord::CLOSED}" &&
                            stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                 },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return testCase.paramValues[0].view->getEntryValue(nullptr) ==
                                 "{EnumWithinRecord::OPEN}" &&
                             stoi(testCase.returnValue.view->getEntryValue(nullptr)) == +1;
                  },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                  } }),
            "enumWithinRecord"
        );
    }

    TEST_F(Syntax_Test, Typedef_Struct_Test) {
        auto [testGen, status] = createTestForFunction(typedefs_1_c, 19);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(typedefs_1_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            auto strParam = testCase.paramValues[0].view->getEntryValue(nullptr);
                            return stoi(strParam.substr(1, strParam.size() - 2)) > 0
                            && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             auto strParam = testCase.paramValues[0].view->getEntryValue(nullptr);
                             return stoi(strParam.substr(1, strParam.size() - 2)) == 0
                             && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             auto strParam = testCase.paramValues[0].view->getEntryValue(nullptr);
                             return stoi(strParam.substr(1, strParam.size() - 2)) < 0
                             && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                         }
                        }),
                "sign_of_typedef_struct");
    }

    TEST_F(Syntax_Test, Typedef_SizeT_Test) {
        auto [testGen, status] = createTestForFunction(typedefs_1_c, 41);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(typedefs_1_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < stoi(testCase.paramValues[1].view->getEntryValue(nullptr))
                            && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == stoi(testCase.paramValues[0].view->getEntryValue(nullptr));
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >= stoi(testCase.paramValues[1].view->getEntryValue(nullptr))
                             && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == stoi(testCase.paramValues[1].view->getEntryValue(nullptr));
                         }
                        }),
                "min_size_t");
    }


    TEST_F(Syntax_Test, Typedef_For_Size_t_Test) {
        auto [testGen, status] = createTestForFunction(typedefs_1_c, 47);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(typedefs_1_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < stoi(testCase.paramValues[1].view->getEntryValue(nullptr))
                            && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == stoi(testCase.paramValues[0].view->getEntryValue(nullptr));
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >= stoi(testCase.paramValues[1].view->getEntryValue(nullptr))
                             && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == stoi(testCase.paramValues[1].view->getEntryValue(nullptr));
                         }
                        }),
                "min_size_t_alias");
    }

    TEST_F(Syntax_Test, Typedef_Enum_Test_1) {
        auto [testGen, status] = createTestForFunction(typedefs_2_c, 13);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(typedefs_2_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "NEG1" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "ZER1" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "POS1" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                         }
                        }
                )
        );
    }

    TEST_F(Syntax_Test, Typedef_Enum_Test_2) {
        auto [testGen, status] = createTestForFunction(typedefs_2_c, 43);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(typedefs_2_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0 && testCase.returnValue.view->getEntryValue(nullptr) == "NEG2";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0 && testCase.returnValue.view->getEntryValue(nullptr) == "ZER2";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) > 0 && testCase.returnValue.view->getEntryValue(nullptr) == "POS2";
                         }
                        }
                )
        );
    }

    TEST_F(Syntax_Test, Packed_Structs_Test_1) {
        auto [testGen, status] = createTestForFunction(packed_structs_c, 10);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(packed_structs_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getSubViews().back()->getEntryValue(nullptr)) > 0
                                && testCase.returnValue.view->getEntryValue(nullptr) == "1";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getSubViews().back()->getEntryValue(nullptr)) < 0
                                && testCase.returnValue.view->getEntryValue(nullptr) == "-1";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getSubViews().back()->getEntryValue(nullptr)) == 0
                                && testCase.returnValue.view->getEntryValue(nullptr) == "0";
                         }
                        }
                )
        );
    }

    TEST_F(Syntax_Test, Packed_Structs_Test_2) {
        auto [testGen, status] = createTestForFunction(packed_structs_c, 24);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(packed_structs_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '1');
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {

                             return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '2');
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {

                             return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr),'3');
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr),'4');
                         }
                        }
                )
        );
    }

    TEST_F(Syntax_Test, Constants_Test_Unsigned_Int_Max) {
        auto [testGen, status] = createTestForFunction(constants_c, 50);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(constants_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "4294967295U" && testCase.returnValue.view->getEntryValue(nullptr) == "true";
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) != "4294967295U" && testCase.returnValue.view->getEntryValue(nullptr) == "false";
                         }
                        }
                ),
                "is_unsigned_int_max"
        );
    }

    TEST_F(Syntax_Test, Constants_Test_Long_Long_Max) {
        auto [testGen, status] = createTestForFunction(constants_c, 57);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(constants_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "9223372036854775807LL" && testCase.returnValue.view->getEntryValue(nullptr) == "true";
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) != "9223372036854775807LL" && testCase.returnValue.view->getEntryValue(nullptr) == "false";
                         }
                        }
                ),
                "is_long_long_max"
        );
    }

    TEST_F(Syntax_Test, Constants_Test_Long_Long_Min) {
        auto [testGen, status] = createTestForFunction(constants_c, 64);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(constants_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "(-9223372036854775807LL - 1)" && testCase.returnValue.view->getEntryValue(nullptr) == "true";
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) != "(-9223372036854775807LL - 1)" && testCase.returnValue.view->getEntryValue(nullptr) == "false";
                         }
                        }
                ),
                "is_long_long_min"
        );
    }

    TEST_F(Syntax_Test, Constants_Test_Unsigned_Long_Long_Max) {
        auto [testGen, status] = createTestForFunction(constants_c, 71);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(constants_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "18446744073709551615ULL" && testCase.returnValue.view->getEntryValue(nullptr) == "true";
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) != "18446744073709551615ULL" && testCase.returnValue.view->getEntryValue(nullptr) == "false";
                         }
                        }
                ),
                "is_unsigned_long_long_max"
        );
    }

    TEST_F(Syntax_Test, Packed_Structs_Test_3) {
        auto [testGen, status] = createTestForFunction(packed_structs_c, 38);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(packed_structs_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return testCase.returnValue.view->getEntryValue(nullptr) == "0";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {

                             return testCase.returnValue.view->getEntryValue(nullptr) == "5";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.returnValue.view->getEntryValue(nullptr) == "-1";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return testCase.returnValue.view->getEntryValue(nullptr) == testCase.paramValues[0].view->getSubViews()[3]->getEntryValue(nullptr);
                         }
                        }
                )
        );

    }

    TEST_F(Syntax_Test, Void_Functions_1) {
        auto [testGen, status] = createTestForFunction(void_functions_c, 12);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(void_functions_c).methods.begin().value().testCases,
                    std::vector<TestCasePredicate>(
                            {[] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0;
                            },
                             [] (const tests::Tests::MethodTestCase& testCase) {
                                 return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) > 0;
                             },
                             [] (const tests::Tests::MethodTestCase& testCase) {
                                 return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0;
                             }
                            })
                );
    }

    TEST_F(Syntax_Test, Void_Functions_2) {
        auto [testGen, status] = createTestForFunction(void_functions_c, 24);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(void_functions_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getSubViews()[0]->getEntryValue(nullptr)) * stoi(testCase.paramValues[1].view->getSubViews()[0]->getEntryValue(nullptr)) < 0;
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getSubViews()[0]->getEntryValue(nullptr)) * stoi(testCase.paramValues[1].view->getSubViews()[0]->getEntryValue(nullptr)) > 0;
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getSubViews()[0]->getEntryValue(nullptr)) * stoi(testCase.paramValues[1].view->getSubViews()[0]->getEntryValue(nullptr)) == 0;
                         }
                        })
        );
    }

    TEST_F(Syntax_Test, Void_Functions_3) {
        auto [testGen, status] = createTestForFunction(void_functions_c, 30);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(void_functions_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return true;
                }})
        );
    }

    TEST_F(Syntax_Test, Void_Functions_4) {
        auto [testGen, status] = createTestForFunction(void_functions_c, 34);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(void_functions_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 6;
                }
                })
        );
    }

    TEST_F(Syntax_Test, Return_Const_Char_Pointer_1) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 56);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), 'a');
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), 'b');
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Return_Const_Char_Pointer_2) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 62);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), 'a');
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), 'b');
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Return_Const_Struct_Pointer_1) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 71);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                      stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) < stoi(testCase.returnValue.view->getSubViews()[1]->getEntryValue(nullptr));
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >= stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                          stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) >= stoi(testCase.returnValue.view->getSubViews()[1]->getEntryValue(nullptr));
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Return_Int_Array) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 87);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == 5;
                }
                })
        );
    }

    TEST_F(Syntax_Test, Return_Void2D) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 100);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, Return_Null_Pointer) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 104);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return testCase.returnValue.view->getEntryValue(nullptr) == "5";
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return testCase.returnValue.view->getEntryValue(nullptr) == "9";
                 },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                    return testCase.returnValue.view->getEntryValue(nullptr) == PrinterUtils::C_NULL;
                  }
                })
        );
    }

    TEST_F(Syntax_Test, Return_Null_Struct) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 116);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return testCase.returnValue.view->getEntryValue(nullptr) == PrinterUtils::C_NULL;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return testCase.returnValue.view->getEntryValue(nullptr) == PrinterUtils::C_NULL;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Restrict_Modifier) {
        auto [testGen, status] = createTestForFunction(qualifiers_c, 22);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(qualifiers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return testCase.paramValues[0].view->getEntryValue(nullptr) == "\"hello\"" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return testCase.paramValues[0].view->getEntryValue(nullptr) !=  "\"hello\"" && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Const_Modifier) {
        auto [testGen, status] = createTestForFunction(qualifiers_c, 38);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(qualifiers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '-');
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) > 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '1');
                 },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '0');
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Volatile_Modifier) {
        auto [testGen, status] = createTestForFunction(qualifiers_c, 49);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(qualifiers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '-');
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) > 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '1');
                 },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0 && testUtils::cmpChars(testCase.returnValue.view->getEntryValue(nullptr), '0');
                 }
                })
        );
    }

    TEST_F(Syntax_Test, CVR_Modifiers) {
        auto [testGen, status] = createTestForFunction(qualifiers_c, 61);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(qualifiers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                    stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) &&
                      stoi(testCase.returnValue.view->getSubViews()[1]->getEntryValue(nullptr)) == stoi(testCase.paramValues[1].view->getEntryValue(nullptr));
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >= stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                          stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                          stoi(testCase.returnValue.view->getSubViews()[1]->getEntryValue(nullptr)) == stoi(testCase.paramValues[0].view->getEntryValue(nullptr));
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Pointers_In_Structs_1) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 10);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.paramValues[0].view->getSubViews()[0]->getEntryValue(nullptr)) > 0 &&
                            stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getSubViews()[0]->getEntryValue(nullptr)) <= 0 &&
                          stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Pointers_In_Structs_2) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 21);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, DISABLED_Pointers_In_Structs_3) {
        //This test worked with flag --search=dfs, but plugin utbot doesn't use this flag
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 31);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[](const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                 },
                  [](const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                  },
                  [](const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1; }
                })
        );
    }

    TEST_F(Syntax_Test, Array_Pointers_In_Struct) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 94);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, Many_Pointers_In_Struct) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 104);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, Complex_Struct) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 108);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                    return testCase.lazyReferences.size() >= 3;
                }
                })
        );
    }

    TEST_F(Syntax_Test, Check_Lazy_Pointers_In_Struct) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 82);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                     return !testCase.lazyReferences.empty();
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Check_Lazy_Pointers_In_Struct_As_Param) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 82);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                     return !testCase.lazyReferences.empty();
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Check_Lazy_Double_Pointers_In_Struct) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 90);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                     return testCase.lazyReferences.size() >= 2;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Check_Lazy_Struct_With_Struct_With_Pointers) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 98);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                     return testCase.lazyReferences.size() >= 3;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Function_Pointers_Base) {
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 10);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(functions_as_params_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 8;
                 },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Function_Pointers_PointerParam) {
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 19);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(functions_as_params_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return testCase.returnValue.view->getEntryValue(nullptr) == "'\\0'";
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return testCase.returnValue.view->getEntryValue(nullptr) == "'\\0'";
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Function_Pointers_StructParam) {
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 28);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(functions_as_params_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 6;
                },
                [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                }
                })
        );
    }

    TEST_F(Syntax_Test, Function_Pointers_StructPointerParam) {
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 40);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(functions_as_params_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 12;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                 },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Correct_CodeText_For_Regression_And_Error) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 7);
        const std::string code = testGen.tests.begin()->second.code;
        const std::string beginRegressionRegion = "#pragma region " + Tests::DEFAULT_SUITE_NAME + NL;
        const std::string endRegion = std::string("#pragma endregion") + NL;
        const std::string beginErrorRegion = "#pragma region " + Tests::ERROR_SUITE_NAME + NL;
        ASSERT_TRUE(code.find(beginRegressionRegion) != std::string::npos) << "No regression begin region";
        ASSERT_TRUE(code.find(endRegion) != std::string::npos) << "No regression end region";
        ASSERT_TRUE(code.find(beginErrorRegion) != std::string::npos) << "No error begin region";
    }

    TEST_F(Syntax_Test, Function_Pointers_StructFieldParam) {
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 52);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(functions_as_params_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Function_Pointers_StructFieldParamTypedefParam) {
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 73);

        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_EQ(1, testUtils::getNumberOfTests(testGen.tests));
    }

    TEST_F(Syntax_Test, Variadic_Test) {
        auto [testGen, status] = createTestForFunction(variadic_c, 8);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(variadic_c).methods.begin().value().testCases, 3);
    }

    TEST_F(Syntax_Test, Struct_with_Char_Pointer) {
        auto [testGen, status] = createTestForFunction(types_c, 62);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Recursive_Struct) {
        auto [testGen, status] = createTestForFunction(types_c, 66);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Struct_With_Const_Pointer_Return) {
        auto [testGen, status] = createTestForFunction(types_c, 87);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Struct_With_Const_Pointer_Return_Pointer) {
        auto [testGen, status] = createTestForFunction(types_c, 107);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Struct_Const_Pointer_Param) {
        auto [testGen, status] = createTestForFunction(types_c, 113);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Multi_Array_1) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 27);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(multi_arrays_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[](const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1 &&
                         testCase.paramValues.front().view->getEntryValue(nullptr) ==
                         testCase.paramPostValues.front().view->getEntryValue(nullptr);
                },
                 [](const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0 &&
                          testCase.paramValues.front().view->getEntryValue(nullptr) ==
                          testCase.paramPostValues.front().view->getEntryValue(nullptr);
                 },
                 [](const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1 &&
                          testCase.paramValues.front().view->getEntryValue(nullptr) ==
                          testCase.paramPostValues.front().view->getEntryValue(nullptr); }
                })
        );
    }

    TEST_F(Syntax_Test, Multi_Pointer_1) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 68);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(multi_arrays_c).methods.begin().value().testCases, 2);
    }


    TEST_F(Syntax_Test, Struct_With_Multi_Array) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 80);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(multi_arrays_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[](const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                },
                 [](const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                 },
                 [](const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1; }
                })
        );
    }

    TEST_F(Syntax_Test, Multi_Pointer_Struct) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 120);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(multi_arrays_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                 },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 3;
                 },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 4;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Return_Struct_With_Array) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 135);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(multi_arrays_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) > 0 &&
                      testCase.returnValue.view->getEntryValue(nullptr) == "{{{1, 2, 3, 4, 5}, {1, 2, 3, 4, 5}}}";
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) < 0 &&
                       testCase.returnValue.view->getEntryValue(nullptr) == "{{{-1, -2, -3, -4, -5}, {-1, -2, -3, -4, -5}}}";
                 },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                   return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0 &&
                          testCase.returnValue.view->getEntryValue(nullptr) == "{{{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}}";
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Sum_Matrix) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 154);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(multi_arrays_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                     return stoi(testCase.returnValue.view->getEntryValue(nullptr)) < 0;
                 },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                  },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) > 0;
                  } }));
    }

    TEST_F(Syntax_Test, Count_Dashes) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 174);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(multi_arrays_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                  return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                },
                  [](const tests::Tests::MethodTestCase &testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) > 0;
                  },
                  [](const tests::Tests::MethodTestCase &testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                  } }));
    }

    TEST_F(Syntax_Test, Floats_Special_Values_Nanf) {
        auto [testGen, status] = createTestForFunction(floats_special_c, 10);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(floats_special_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase &testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "NAN";
                        }}));
    }

    TEST_F(Syntax_Test, Floats_Special_Values_Nan) {
        auto [testGen, status] = createTestForFunction(floats_special_c, 18);


        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
                testGen.tests.at(floats_special_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase &testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "NAN";
                        }}));
    }

    TEST_F(Syntax_Test, Floats_Special_Values_Inf) {
        auto [testGen, status] = createTestForFunction(floats_special_c, 27);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(floats_special_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                    { [](const tests::Tests::MethodTestCase &testCase) {
                        return testCase.paramValues[0].view->getEntryValue(nullptr) == "INFINITY";
                    } }));
    }

    TEST_F(Syntax_Test, Accept_Const_Int_Const_Pointer_Const_Pointer) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 186);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Accept_Const_Int_Const_Pointer_Pointer) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 190);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Accept_Const_Int_Pointer_Pointer) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 194);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 1);
    }


    TEST_F(Syntax_Test, Supported_2d_Pointer) {
        auto [testGen, status] = createTestForFunction(types_c, 72);

        ASSERT_TRUE(status.ok()) << status.error_message();

        int numberOfTests = testUtils::getNumberOfTests(testGen.tests);
        EXPECT_EQ(2, numberOfTests);
    }

    TEST_F(Syntax_Test, Supported_Void_Pointer) {
        auto [testGen, status] = createTestForFunction(types_c, 77);

        ASSERT_TRUE(status.ok()) << status.error_message();

        int numberOfTests = testUtils::getNumberOfTests(testGen.tests);
        EXPECT_EQ(1, numberOfTests);
    }

    TEST_F(Syntax_Test, Support_Struct_with_Union1) {
        auto [testGen, status] = createTestForFunction(struct_with_union_c, 7);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(struct_with_union_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 0 &&
                                    StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                            "{from_bytes<StructWithUnion::InnerUnion>({17, 0, 0, 0}), "
                                                            "{from_bytes<StructWithUnion::InnerStructWithUnion::Inner2Union>({48,")
                                    && StringUtils::endsWith(testCase.returnValue.view->getEntryValue(nullptr), "})}, -108}");
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0 &&
                                    StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                            "{from_bytes<StructWithUnion::InnerUnion>({97,")
                                    && StringUtils::endsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                             "}), {from_bytes<StructWithUnion::InnerStructWithUnion::Inner2Union>({101, 0, 0, 0})}, 155}");
                         }
                        })
        );
    }

    TEST_F(Syntax_Test, Support_Struct_with_Union2) {
        auto [testGen, status] = createTestForFunction(struct_with_union_c, 21);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(struct_with_union_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) +
                                   stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) < 0 &&
                                   StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                           "{from_bytes<StructWithUnionInUnion::Union1>({98,");
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) +
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) >= 0 &&
                                    stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) +
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) <= 16 &&
                                    testCase.returnValue.view->getEntryValue(nullptr) ==
                                            "{from_bytes<StructWithUnionInUnion::Union1>({-113, -62, -11, 40, 92, -113, -10, 63})}";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) +
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) > 16 &&
                                    StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                            "{from_bytes<StructWithUnionInUnion::Union1>({-5, -1, -1, -1,");
                         }
                        })
        );
    }

    TEST_F(Syntax_Test, Support_Struct_with_Union3) {
        auto [testGen, status] = createTestForFunction(struct_with_union_c, 33);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(struct_with_union_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) <
                                   stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                   StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                           "{from_bytes<StructWithStructInUnion::DeepUnion>({-103, 0, 0, 0, 0, 0, 0, 0,");
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) ==
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                    StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                            "{from_bytes<StructWithStructInUnion::DeepUnion>({107,") &&
                                    StringUtils::endsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                          "-102, 8, 27, -98, 94, 41, -16, 63})}");
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                    StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                            "{from_bytes<StructWithStructInUnion::DeepUnion>({0, 0, 0, 0, 0, 0, 0, 0,");
                         }
                        })
        );
    }

    TEST_F(Syntax_Test, length_of_linked_list3) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 7);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(linked_list_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {
                            [] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                                },
                            [] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                                },
                            [] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 3;
                                },
                            [] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                            }
                        })
        );
    }

    TEST_F(Syntax_Test, length_of_linked_list2) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 23);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(linked_list_c).methods.begin().value().testCases,
                    std::vector<TestCasePredicate>(
                        {
                            [] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                                },
                            [] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                                },
                            [] (const tests::Tests::MethodTestCase& testCase) {
                                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                            }
                        })
        );
    }

    TEST_F(Syntax_Test, hard_length_of_linked_list2) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 36);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(linked_list_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, middle_length_of_linked_list2) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 49);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(linked_list_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                                }
                        })
        );
    }

    TEST_F(Syntax_Test, cycle_linked_list3) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 62);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(linked_list_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 3;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 4;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 5;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 6;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -2;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -3;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 17;
                                }
                        })
        );
    }

    TEST_F(Syntax_Test, len_bound) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 96);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(linked_list_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) > -1;
                }
            })
        );
    }

    TEST_F(Syntax_Test, sort_list) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 108);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(linked_list_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                }
            })
        );
    }

    TEST_F(Syntax_Test, sort_list_with_cmp) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 139);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(linked_list_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, sort_array) {
        auto [testGen, status] = createTestForFunction(array_sort_c, 9);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(array_sort_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, sort_array_with_comparator) {
        auto [testGen, status] = createTestForFunction(array_sort_c, 37);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(array_sort_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, find_maximum) {
        auto [testGen, status] = createTestForFunction(stubs_c, 7);

        ASSERT_TRUE(status.ok()) << status.error_message();

        EXPECT_EQ(2, testUtils::getNumberOfTests(testGen.tests));
    }

    TEST_F(Syntax_Test, vowel_consonant) {
        auto [testGen, status] = createTestForFunction(stubs_c, 16);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(stubs_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, tree_deep) {
        auto [testGen, status] = createTestForFunction(tree_c, 7);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(tree_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                                },
                                [] (const tests::Tests::MethodTestCase& testCase) {
                                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                                }
                        })
        );
    }

    TEST_F(Syntax_Test, UnnamedTypeUnionField) {
        auto [_, status] = createTestForFunction(types_3_c, 19);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, UnnamedTypeStructField) {
        auto [testGen, status] = createTestForFunction(types_3_c, 37);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 2);
    }

    TEST_F(Syntax_Test, AnonymousUnionField) {
        auto [testGen, status] = createTestForFunction(types_3_c, 52);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, AnonymousStructField) {
        auto [testGen, status] = createTestForFunction(types_3_c, 69);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 2);
    }

    TEST_F(Syntax_Test, Vector_Sum) {
        auto [testGen, status] = createTestForFunction(types_3_c, 80);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Vector_Create) {
        auto [testGen, status] = createTestForFunction(types_3_c, 95);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Accept_Incomplete) {
        auto [testGen, status] = createTestForFunction(types_3_c, 105);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Return_Incomplete) {
        auto [testGen, status] = createTestForFunction(types_3_c, 109);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Pass_Forward_Decl) {
        auto [testGen, status] = createTestForFunction(types_3_c, 116);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Duplicate_Struct) {
        auto [testGen, status] = createTestForFunction(types_3_c, 132);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 3);
    }

    TEST_F(Syntax_Test, Global_Unnamed_Variable) {
        auto [testGen, status] = createTestForFunction(types_3_c, 149);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(types_3_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
              EXPECT_TRUE(testCase.globalPostValues.empty());
              return testCase.returnValue.view->getEntryValue(nullptr) == "-1";
            } }),
            "check_option");
    }

    TEST_F(Syntax_Test, Run_Tests_For_Linked_List) {
        auto request = testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath,
                                                    srcPaths, linked_list_c, true, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        testGen.setTargetForSource(linked_list_c);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_GE(testUtils::getNumberOfTests(testGen.tests), 2);

        fs::path testsDirPath = getTestFilePath("tests");

        fs::path linked_list_test_cpp = Paths::sourcePathToTestPath(utbot::ProjectContext(
            projectName, suitePath, testsDirPath, buildDirRelativePath), linked_list_c);
        auto testFilter = GrpcUtils::createTestFilterForFile(linked_list_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
            std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(), coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{ true, false, 45, 0, false, false };
        coverageGenerator.generate(false, settingsContext);

        EXPECT_FALSE(coverageGenerator.hasExceptions());
        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto statusMap = coverageGenerator.getTestStatusMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        // TODO: add checkStatusesCount after linkedlist fix
        testUtils::checkStatuses(statusMap, tests);
    }

    TEST_F(Syntax_Test, Run_Tests_For_Tree) {
        auto request = testUtils::createFileRequest(projectName, suitePath, buildDirRelativePath,
                                                    srcPaths, tree_c, true, false);
        auto testGen = FileTestGen(*request, writer.get(), TESTMODE);
        testGen.setTargetForSource(tree_c);
        Status status = Server::TestsGenServiceImpl::ProcessBaseTestRequest(testGen, writer.get());
        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_GE(testUtils::getNumberOfTests(testGen.tests), 2);

        fs::path testsDirPath = getTestFilePath("tests");

        fs::path tree_test_cpp = Paths::sourcePathToTestPath(utbot::ProjectContext(
            projectName, suitePath, testsDirPath, buildDirRelativePath), tree_c);
        auto testFilter = GrpcUtils::createTestFilterForFile(tree_test_cpp);
        auto runRequest = testUtils::createCoverageAndResultsRequest(
            projectName, suitePath, testsDirPath, buildDirRelativePath, std::move(testFilter));

        static auto coverageAndResultsWriter =
            std::make_unique<ServerCoverageAndResultsWriter>(nullptr);
        CoverageAndResultsGenerator coverageGenerator{ runRequest.get(), coverageAndResultsWriter.get() };
        utbot::SettingsContext settingsContext{ true, false, 15, 0, false, false };
        coverageGenerator.generate(false, settingsContext);

        EXPECT_FALSE(coverageGenerator.hasExceptions());
        ASSERT_TRUE(coverageGenerator.getCoverageMap().empty());

        auto statusMap = coverageGenerator.getTestStatusMap();
        auto tests = coverageGenerator.getTestsToLaunch();

        testUtils::checkStatuses(statusMap, tests);

        StatusCountMap expectedStatusCountMap{
            {testsgen::TEST_DEATH, 4},
            {testsgen::TEST_PASSED, 6}};
        testUtils::checkStatusesCount(statusMap, tests, expectedStatusCountMap);
    }

    TEST_F(Syntax_Test, Simple_parameter_cpp) {
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 8);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
              testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.empty();
                      }}),
              "simple_parameter_cpp");
    }

    TEST_F(Syntax_Test, Pointer_parameter_cpp) {
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 15);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
              testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramPostValues[0].view->getEntryValue(nullptr)) == stoi(testCase.returnValue.view->getEntryValue(nullptr));
                       },
                       [] (const tests::Tests::MethodTestCase& testCase) {
                         return stoi(testCase.paramPostValues[0].view->getEntryValue(nullptr)) == stoi(testCase.returnValue.view->getEntryValue(nullptr));
                       }
                      }),
              "pointer_parameter_cpp");
    }

    TEST_F(Syntax_Test, Double_pointer_parameter_cpp) {
        auto[testGen, status] = createTestForFunction(different_parameters_cpp, 23);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
                testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase &testCase) {
                            return stoi(
                                    testCase.paramPostValues[0].view->getSubViews().front()->getSubViews().front()->getEntryValue(nullptr)) ==
                                   stoi(testCase.returnValue.view->getEntryValue(nullptr));
                        },
                         [](const tests::Tests::MethodTestCase &testCase) {
                             return stoi(
                                     testCase.paramPostValues[0].view->getSubViews().front()->getSubViews().front()->getEntryValue(nullptr)) ==
                                    stoi(testCase.returnValue.view->getEntryValue(nullptr));
                         }
                        }),
                "Double_pointer_parameter_cpp");
    }

    TEST_F(Syntax_Test, Lvalue_parameter_cpp) {
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 29);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
              testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramPostValues[0].view->getEntryValue(nullptr)) == stoi(testCase.returnValue.view->getEntryValue(nullptr));
                        },
                       [] (const tests::Tests::MethodTestCase& testCase) {
                         return stoi(testCase.paramPostValues[0].view->getEntryValue(nullptr)) == stoi(testCase.returnValue.view->getEntryValue(nullptr));
                        }
                      }),
              "lvalue_parameter");
    }

    TEST_F(Syntax_Test, Const_parameter_cpp) {
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 43);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
              testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.empty();
                      }}),
              "const_parameter_cpp");
    }

    TEST_F(Syntax_Test, Const_pointer_parameter_cpp) {
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 50);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
              testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.empty();
                      }}),
              "const_pointer_parameter_cpp");
    }

    TEST_F(Syntax_Test, Const_double_pointer_parameter_cpp) {
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 57);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
              testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.empty();
                      }}),
              "const_double_pointer_parameter_cpp");
    }

    TEST_F(Syntax_Test, Const_lvalue_parameter_cpp) {
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 64);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
              testGen.tests.at(different_parameters_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.empty();
                      }}),
              "const_lvalue_parameter_cpp");
    }

    TEST_F(Syntax_Test, Simple_getter_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 20);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Operator_plus_eq_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 28);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Operator_plus_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 34);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Change_class_by_ref_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 38);

        ASSERT_TRUE(status.ok()) << status.error_message();
        printer::TestsPrinter testsPrinter(nullptr, utbot::Language::CXX);
        const auto &tests = testGen.tests.at(simple_class_cpp)
                                .methods.begin().value().testCases.begin();
        ASSERT_EQ("{"
                  "\n    /*.x = */-1,"
                  "\n    /*.y = */2}", tests[0].paramValues[0].view->getEntryValue(&testsPrinter));

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 5);

        checkTestCasePredicates(
              testGen.tests.at(simple_class_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.front().view->getSubViews()[0]->getEntryValue(nullptr) == "0" &&
                               testCase.paramPostValues.front().view->getSubViews()[1]->getEntryValue(nullptr) == "0";
                      }, [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.front().view->getSubViews()[0]->getEntryValue(nullptr) == "0" &&
                               testCase.paramPostValues.front().view->getSubViews()[1]->getEntryValue(nullptr) == "0";
                      }, [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.front().view->getSubViews()[0]->getEntryValue(nullptr) == "0" &&
                               testCase.paramPostValues.front().view->getSubViews()[1]->getEntryValue(nullptr) == "0";
                      }, [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.front().view->getSubViews()[0]->getEntryValue(nullptr) == "0" &&
                               testCase.paramPostValues.front().view->getSubViews()[1]->getEntryValue(nullptr) == "0";
                      }, [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.paramPostValues.front().view->getSubViews()[0]->getEntryValue(nullptr) == "0" &&
                               testCase.paramPostValues.front().view->getSubViews()[1]->getEntryValue(nullptr) == "0";
                      }
                      }),
              "change_class_by_ref_cpp");
    }

    TEST_F(Syntax_Test, Change_class_by_ref_2_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 54);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 3);

        checkTestCasePredicates(
              testGen.tests.at(simple_class_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramPostValues.front().view->getSubViews()[0]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.paramValues.front().view->getSubViews()[0]->getEntryValue(nullptr))) &&
                               stoi(testCase.paramPostValues.front().view->getSubViews()[1]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.paramValues.front().view->getSubViews()[1]->getEntryValue(nullptr)));
                      }, [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramPostValues.front().view->getSubViews()[0]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.paramValues.front().view->getSubViews()[0]->getEntryValue(nullptr))) &&
                               stoi(testCase.paramPostValues.front().view->getSubViews()[1]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.paramValues.front().view->getSubViews()[0]->getEntryValue(nullptr)));
                      }, [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramPostValues.front().view->getSubViews()[0]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.paramValues.front().view->getSubViews()[0]->getEntryValue(nullptr))) &&
                               stoi(testCase.paramPostValues.front().view->getSubViews()[1]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.paramValues.front().view->getSubViews()[0]->getEntryValue(nullptr)));
                      }
                      }),
              "change_class_by_ref_2_cpp");
    }

    TEST_F(Syntax_Test, Change_class_by_method_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 64);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 3);

        checkTestCasePredicates(
              testGen.tests.at(simple_class_cpp).methods.begin().value().testCases,
              std::vector<TestCasePredicate>(
                      {[] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.classPostValues.has_value() &&
                               stoi(testCase.classPostValues.value().view->getSubViews()[0]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.classPreValues.value().view->getSubViews()[0]->getEntryValue(nullptr))) &&
                               stoi(testCase.classPostValues.value().view->getSubViews()[1]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.classPreValues.value().view->getSubViews()[1]->getEntryValue(nullptr)));
                      }, [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.classPostValues.has_value() &&
                               stoi(testCase.classPostValues.value().view->getSubViews()[0]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.classPreValues.value().view->getSubViews()[0]->getEntryValue(nullptr))) &&
                               stoi(testCase.classPostValues.value().view->getSubViews()[1]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.classPreValues.value().view->getSubViews()[0]->getEntryValue(nullptr)));
                      }, [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.classPostValues.has_value() &&
                               stoi(testCase.classPostValues.value().view->getSubViews()[0]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.classPreValues.value().view->getSubViews()[0]->getEntryValue(nullptr))) &&
                               stoi(testCase.classPostValues.value().view->getSubViews()[1]->getEntryValue(nullptr))
                                   == abs(stoi(testCase.classPreValues.value().view->getSubViews()[0]->getEntryValue(nullptr)));
                      }
                      }),
              "change_class_by_method_cpp");
    }

    TEST_F(Syntax_Test, Inner_unnamed_union_return) {
        auto[testGen, status] = createTestForFunction(inner_unnamed_c, 8);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(inner_unnamed_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    std::stringstream ss;
                    EXPECT_EQ(testCase.paramValues.front().view->getEntryValue(nullptr).size(), 3);
                    ss << "from_bytes<StructWithUnnamedUnion>({"
                       << int(testCase.paramValues.front().view->getEntryValue(nullptr)[1])
                       << ", 0, 0, 0, "
                       << int(testCase.paramValues.front().view->getEntryValue(nullptr)[1])
                       << ", 0, 0, 0})";
                    return testCase.returnValue.view->getEntryValue(nullptr) == ss.str();
                }}));
    }

    TEST_F(Syntax_Test, Inner_unnamed_union_parameter) {
        auto[testGen, status] = createTestForFunction(inner_unnamed_c, 15);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(inner_unnamed_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    return "from_bytes<StructWithUnnamedUnion>({0, 0, 0, 0, 0, 0, 0, 0})" ==
                           testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "from_bytes<StructWithUnnamedUnion>({42, 0, 0, 0, 42, 0, 0, 0})" ==
                           testCase.returnValue.view->getEntryValue(nullptr);
                }, [](const tests::Tests::MethodTestCase &testCase) {
                    return "from_bytes<StructWithUnnamedUnion>({0, 0, 0, 0, 0, 0, 0, 0})" !=
                           testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "from_bytes<StructWithUnnamedUnion>({24, 0, 0, 0, 24, 0, 0, 0})" ==
                           testCase.returnValue.view->getEntryValue(nullptr);

                }}));
    }

    TEST_F(Syntax_Test, Inner_unnamed_struct_return) {
        auto[testGen, status] = createTestForFunction(inner_unnamed_c, 27);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(inner_unnamed_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    std::stringstream ss;
                    EXPECT_EQ(testCase.paramValues.front().view->getEntryValue(nullptr).size(), 3);
                    ss << "from_bytes<UnionWithUnnamedStruct>({"
                       << int(testCase.paramValues.front().view->getEntryValue(nullptr)[1])
                       << ", 0, 0, 0, "
                       << int(testCase.paramValues.front().view->getEntryValue(nullptr)[1])
                       << ", 0, 0, 0})";
                    return testCase.returnValue.view->getEntryValue(nullptr) == ss.str();
                }}));
    }


    TEST_F(Syntax_Test, Inner_unnamed_struct_parameter) {
        auto[testGen, status] = createTestForFunction(inner_unnamed_c, 33);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(inner_unnamed_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    return "from_bytes<UnionWithUnnamedStruct>({0, 0, 0, 0, 0, 0, 0, 0})" ==
                           testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "from_bytes<UnionWithUnnamedStruct>({42, 0, 0, 0, 42, 0, 0, 0})" ==
                           testCase.returnValue.view->getEntryValue(nullptr);
                }, [](const tests::Tests::MethodTestCase &testCase) {
                    return "from_bytes<UnionWithUnnamedStruct>({0, 0, 0, 0, 0, 0, 0, 0})" !=
                           testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "from_bytes<UnionWithUnnamedStruct>({24, 0, 0, 0, 24, 0, 0, 0})" ==
                           testCase.returnValue.view->getEntryValue(nullptr);

                }}));
    }

    TEST_F(Syntax_Test, Typedef_to_pointer_array) {
        auto[testGen, status] = createTestForFunction(pointer_parameters_c, 43);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(pointer_parameters_c).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
                testGen.tests.at(pointer_parameters_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    return "{{0}, {0}}" == testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "42" == testCase.returnValue.view->getEntryValue(nullptr);
                }, [](const tests::Tests::MethodTestCase &testCase) {
                    return "{{0}, {0}}" != testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "24" == testCase.returnValue.view->getEntryValue(nullptr);
                }
                                          }));
    }

    TEST_F(Syntax_Test, void_ptr) {
        auto [testGen, status] = createTestForFunction(pointer_parameters_c, 49);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(pointer_parameters_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, length_of_empty_list) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 170);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(linked_list_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 3;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, content_of_void_ptr) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 186);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(linked_list_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, example_namespace) {
        auto [testGen, status] = createTestForFunction(namespace_cpp, 7);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(namespace_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return StringUtils::startsWith(testCase.returnValue.view->getSubViews()[2]->getEntryValue(nullptr), "from_bytes<uni::inner1::U>") &&
                             stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == 5;
                    },
                     [] (const tests::Tests::MethodTestCase& testCase) {
                       return StringUtils::startsWith(testCase.returnValue.view->getSubViews()[2]->getEntryValue(nullptr), "from_bytes<uni::inner1::U>") &&
                              stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == -1;
                     },
                     [] (const tests::Tests::MethodTestCase& testCase) {
                       return StringUtils::startsWith(testCase.returnValue.view->getSubViews()[2]->getEntryValue(nullptr), "from_bytes<uni::inner1::U>") &&
                              stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == 10;
                     }
                })
        );
    }

    TEST_F(Syntax_Test, struct_with_union_as_return_type_cpp) {
        auto [testGen, status] = createTestForFunction(namespace_cpp, 28);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(namespace_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return StringUtils::startsWith(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr), "from_bytes<StructWithUnion::InnerUnion>") &&
                             StringUtils::startsWith(testCase.returnValue.view->getSubViews()[1]->getSubViews()[0]->getEntryValue(nullptr),
                                                       "from_bytes<StructWithUnion::InnerStructWithUnion::Inner2Union>") &&
                               stoi(testCase.returnValue.view->getSubViews()[2]->getEntryValue(nullptr)) == -108;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return StringUtils::startsWith(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr), "from_bytes<StructWithUnion::InnerUnion>") &&
                             StringUtils::startsWith(testCase.returnValue.view->getSubViews()[1]->getSubViews()[0]->getEntryValue(nullptr),
                                                     "from_bytes<StructWithUnion::InnerStructWithUnion::Inner2Union>") &&
                             stoi(testCase.returnValue.view->getSubViews()[2]->getEntryValue(nullptr)) == 155;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, multi_union) {
        auto [testGen, status] = createTestForFunction(namespace_cpp, 42);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(namespace_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return StringUtils::startsWith(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr), "from_bytes<A1::B1>") &&
                             StringUtils::startsWith(testCase.returnValue.view->getSubViews()[1]->getEntryValue(nullptr), "from_bytes<A1::C1>");
                    }
                })
        );
    }
}


