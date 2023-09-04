#include "gtest/gtest.h"

#include "BaseTest.h"
#include "KleeGenerator.h"
#include "Server.h"
#include "TestUtils.h"
#include "Tests.h"
#include "coverage/CoverageAndResultsGenerator.h"
#include "gmock/gmock.h"
#include "streams/coverage/ServerCoverageAndResultsWriter.h"
#include "utils/SizeUtils.h"
#include "utils/StringUtils.h"
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
    using namespace ::testsgen;

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
        fs::path different_variables_cpp = getTestFilePath("different_variables.cpp");
        fs::path simple_class_cpp = getTestFilePath("simple_class.cpp");
        fs::path inner_unnamed_c = getTestFilePath("inner_unnamed.c");
        fs::path array_sort_c = getTestFilePath("array_sort.c");
        fs::path constructors_cpp = getTestFilePath("constructors.cpp");
        fs::path stubs_c = getTestFilePath("stubs.c");
        fs::path input_output_c = getTestFilePath("input_output.c");
        fs::path file_c = getTestFilePath("file.c");
        fs::path bitfields_c = getTestFilePath("bitfields.c");
        fs::path namespace_cpp = getTestFilePath("namespace.cpp");
        fs::path rvalue_reference_cpp = getTestFilePath("function_with_rvalue_params.cpp");
        fs::path hard_linked_list_c = getTestFilePath("hard_linked_list.c");
        fs::path unsupported_class_cpp = getTestFilePath("unsupported_class.cpp");

        void SetUp() override {
            clearEnv(CompilationUtils::CompilerName::CLANG);
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
                                                 srcPaths, pathToFile, lineNum, pathToFile,
                                                 false, false, kleeTimeout);
            auto request = GrpcUtils::createFunctionRequest(std::move(lineRequest));
            auto testGen = FunctionTestGen(*request, writer.get(), TESTMODE);
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
        auto [testGen, status] = createTestForFunction(simple_structs_c, 5);

        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_structs_c)
                                .methods.begin().value().testCases;
        testUtils::checkRegexp(tests[0].paramValues[0].view->getEntryValue(&testsPrinter),
                               "[{]"
                               "\n    [.]x = .+,"
                               "\n    [.]a = .+"
                               "\n[}]");

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
                tests,
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
        auto [testGen, status] = createTestForFunction(simple_structs_c, 33);

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
        auto [testGen, status] = createTestForFunction(simple_structs_c, 74);

        ASSERT_TRUE(status.ok()) << status.error_message();

        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_structs_c)
                .methods.begin().value().testCases;
        testUtils::checkRegexp(tests[0].returnValue.view->getEntryValue(&testsPrinter),
                               "[{]"
                               "\n    [.]inner = [{]"
                               "\n        [.]c = '.+',"
                               "\n        [.]ininner = [{]"
                               "\n            [.]u = .+U,"
                               "\n            [.]l = .+LL"
                               "\n        [}],"
                               "\n        [.]s = .+"
                               "\n    [}][,]"
                               "\n    [.]x = .+,"
                               "\n    [.]y = .+LL"
                               "\n[}]");

        checkTestCasePredicates(
                tests,
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
        auto [testGen, status] = createTestForFunction(simple_unions_c, 5);

        ASSERT_TRUE(status.ok()) << status.error_message();

        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_unions_c).methods.begin().value().testCases;
        testUtils::checkRegexp(tests[0].paramValues[0].view->getEntryValue(&testsPrinter),
                               "[{]"
                               "\n    [.]bytes = [{]'.+', '.+', '.+', '.+'[}]"
                               "\n    // [.]number = .+"
                               "\n[}]");

        checkTestCasePredicates(
                tests,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0 &&
                                testCase.paramValues[0].view->getEntryValue(nullptr) == "{{'\\0', '\\0', '\\0', '\\0'}}";
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1 &&
                                testCase.paramValues[0].view->getEntryValue(nullptr) != "{{'\\0', '\\0', '\\0', '\\0'}}";
                         },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1 &&
                                testCase.paramValues[0].view->getEntryValue(nullptr) != "{{'\\0', '\\0', '\\0', '\\0'}}";
                         }
                        }),
                "get_sign_union");
    }

    TEST_F(Syntax_Test, Union_Parameter_Test_2) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 15);

        ASSERT_TRUE(status.ok()) << status.error_message();

        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_unions_c).methods.begin().value().testCases;
        testUtils::checkRegexp(tests[0].paramValues[0].view->getEntryValue(&testsPrinter),
                               "[{]"
                               "\n    [.]bytes = [{]'.+', '.+'[}]"
                               "\n    // [.]number = .+"
                               "\n[}]");


        checkTestCasePredicates(
                tests,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            std::cout << testCase.paramValues[0].view->getEntryValue(nullptr) << std::endl;
                            return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find("{{'\\0', ") == 0;
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find("{{'\\0', ") == std::string::npos;
                         },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1
                                && testCase.paramValues[0].view->getEntryValue(nullptr).find("{{'\\0', ") == 0;
                         }
                        }),
                "extract_bit");
    }

    TEST_F(Syntax_Test, Union_Return_Test) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 74);

        ASSERT_TRUE(status.ok()) << status.error_message();

        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_unions_c).methods.begin().value().testCases;
        testUtils::checkRegexp(tests[0].returnValue.view->getEntryValue(&testsPrinter),
                               "[{]"
                               "\n    [.]inner = [{]"
                               "\n        // [.]c = '.+'"
                               "\n        [.]ininner = [{]"
                               "\n            // [.]u = .+U"
                               "\n            [.]l = .+LL"    // <- folds to {{{[0-9]+LL}}}
                               "\n        [}]"
                               "\n        // [.]s = .+"
                               "\n    }"
                               "\n    // [.]x = .+"
                               "\n    // [.]y = .+LL"
                               "\n[}]");

        checkTestCasePredicates(
                tests,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0
                               && testCase.returnValue.view->getEntryValue(nullptr) == "{{{48LL}}}";
                        },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 1
                               && testCase.returnValue.view->getEntryValue(nullptr) == "{{{1LL}}}";
                         },
                         [](const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 0
                               && stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 1
                                && testCase.returnValue.view->getEntryValue(nullptr) == "{{{2LL}}}";
                         },
                        }),
                "union_as_return_type");
    }

    TEST_F(Syntax_Test, Union_Array_Test) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 102);

        ASSERT_TRUE(status.ok()) << status.error_message();

        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_unions_c).methods.begin().value().testCases;
        testUtils::checkRegexp(tests[0].paramValues[0].view->getEntryValue(&testsPrinter),
                               "[{]([{]"
                               "\n    [.]bytes = [{]'.+', '.+', '.+', '.+'[}]"
                               "\n    // [.]number = .+"
                               "\n[}](, )?)+[}]");

        checkTestCasePredicates(
                tests,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase& testCase) {
                            size_t it = 0;
                            int cnt = 0;
                            auto const &str = testCase.paramValues[0];
                            const char *substr = "'}},";
                            while ((it = str.view->getEntryValue(nullptr).find(substr, it)) != std::string::npos) {
                                cnt++;
                                it++;
                            }
                            return cnt == 10 - 1;
                        }}),
                "sumOfUnionArray");
    }

    TEST_F(Syntax_Test, Union_With_Pointer_Test) {
        auto [testGen, status] = createTestForFunction(simple_unions_c, 112);

        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(simple_unions_c).methods.begin().value().testCases;
        testUtils::checkRegexp(tests[0].paramValues[0].view->getEntryValue(&testsPrinter),
                               "[{]"
                               "\n    [.]a = .+" // NULL or (int *) ...
                               "\n    // [.]b = .+LL"
                               "\n[}]");


        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            tests,
            std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
                return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
            } }),
            "operateWithUnionWithPointer");
    }

    TEST_F(Syntax_Test, Pointer_Return_Test_1) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 8);

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
        auto [testGen, status] = createTestForFunction(pointer_return_c, 40);

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
        auto [testGen, status] = createTestForFunction(pointer_return_c, 79);

        ASSERT_TRUE(status.ok()) << status.error_message();
        checkTestCasePredicates(
            testGen.tests.at(pointer_return_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                auto entryValue = testCase.paramValues[0].view->getEntryValue(nullptr);
                auto returnValue = stoll(testCase.returnValue.view->getEntryValue(nullptr));
                return static_cast<unsigned char>(entryValue[2]) ==
                       static_cast<unsigned char>(returnValue);
            }
                }),
            "void_pointer_return_char_usage");
    }

    TEST_F(Syntax_Test, Return_Long_Long_Array) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 90);

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
        auto [testGen, status] = createTestForFunction(pointer_parameters_c, 30);

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
        auto [testGen, status] = createTestForFunction(complex_structs_c, 7);

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
        auto [testGen, status] = createTestForFunction(complex_structs_c, 39);

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
        auto [testGen, status] = createTestForFunction(complex_structs_c, 54);

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
        auto [testGen, status] = createTestForFunction(types_c, 46);

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
        auto [testGen, status] = createTestForFunction(types_c, 52);

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
        auto [testGen, status] = createTestForFunction(enums_c, 7);

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
        auto [testGen, status] = createTestForFunction(pointer_parameters_c, 24);

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
        auto [testGen, status] = createTestForFunction(enums_c, 39);

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
        auto [testGen, status] = createTestForFunction(enums_c, 18);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkReturnEnum(testGen);
    }

    TEST_F(Syntax_Test, Enum_Pointer_as_Return_Test) {
        auto [testGen, status] = createTestForFunction(enums_c, 43);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkReturnEnum(testGen);
    }


    TEST_F(Syntax_Test, Enum_in_Struct_Test) {
        auto [testGen, status] = createTestForFunction(enums_c, 26);

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
        auto [testGen, status] = createTestForFunction(enums_c, 51);

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
        auto [testGen, status] = createTestForFunction(enums_c, 69);


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

    TEST_F(Syntax_Test, Anonymous_Enum_As_Return_Test) {
        auto [testGen, status] = createTestForFunction(enums_c, 79);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(enums_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[](const tests::Tests::MethodTestCase& testCase) {
                      return testCase.returnValue.view->getEntryValue(nullptr) == "EVEN" && stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) % 2 == 0;
                  },
                 [](const tests::Tests::MethodTestCase& testCase) {
                      return testCase.returnValue.view->getEntryValue(nullptr) == "ODD"  && stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) % 2 == 1;
                  }
                }
            ),
            "intToParity"
        );
    }

    TEST_F(Syntax_Test, Typedef_Struct_Test) {
        auto [testGen, status] = createTestForFunction(typedefs_1_c, 15);

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
        auto [testGen, status] = createTestForFunction(typedefs_1_c, 37);

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
        auto [testGen, status] = createTestForFunction(typedefs_1_c, 43);

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
        auto [testGen, status] = createTestForFunction(typedefs_2_c, 9);

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
        auto [testGen, status] = createTestForFunction(typedefs_2_c, 39);

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
        auto [testGen, status] = createTestForFunction(packed_structs_c, 6);

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
        auto [testGen, status] = createTestForFunction(packed_structs_c, 20);

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
        auto [testGen, status] = createTestForFunction(constants_c, 46);


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
        auto [testGen, status] = createTestForFunction(constants_c, 52);


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
        auto [testGen, status] = createTestForFunction(constants_c, 60);


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
        auto [testGen, status] = createTestForFunction(constants_c, 67);


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
        auto [testGen, status] = createTestForFunction(packed_structs_c, 34);

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
        auto [testGen, status] = createTestForFunction(void_functions_c, 8);

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
        auto [testGen, status] = createTestForFunction(void_functions_c, 20);

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
        auto [testGen, status] = createTestForFunction(void_functions_c, 26);

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
        auto [testGen, status] = createTestForFunction(void_functions_c, 30);

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
        auto [testGen, status] = createTestForFunction(pointer_return_c, 52);

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
        auto [testGen, status] = createTestForFunction(pointer_return_c, 58);

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
        auto [testGen, status] = createTestForFunction(pointer_return_c, 67);

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
        auto [testGen, status] = createTestForFunction(pointer_return_c, 83);

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
        auto [testGen, status] = createTestForFunction(pointer_return_c, 96);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, Return_Null_Pointer) {
        auto [testGen, status] = createTestForFunction(pointer_return_c, 100);

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
        auto [testGen, status] = createTestForFunction(pointer_return_c, 112);

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
        auto [testGen, status] = createTestForFunction(qualifiers_c, 18);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(qualifiers_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                  auto param_values = testCase.paramValues[0].view->getEntryValue(nullptr);
                  const int word_end = 31; //End of word "hello" in param_values string
                  return param_values == ("{'h', 'e', 'l', 'l', 'o', '\\0'," + param_values.substr(word_end))  && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                },
                 [] (const tests::Tests::MethodTestCase& testCase) {
                      auto param_values = testCase.paramValues[0].view->getEntryValue(nullptr);
                      const int word_end = 31; //End of word "hello" in param_values string
                      return param_values != ("{'h', 'e', 'l', 'l', 'o', '\\0'," + param_values.substr(word_end)) && stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                 }
                })
        );
    }

    TEST_F(Syntax_Test, Const_Modifier) {
        auto [testGen, status] = createTestForFunction(qualifiers_c, 34);

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
        auto [testGen, status] = createTestForFunction(qualifiers_c, 45);

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
        auto [testGen, status] = createTestForFunction(qualifiers_c, 57);

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
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 6);

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
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 17);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(structs_with_pointers_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Pointers_In_Structs_3) {
        //This test worked with flag --search=dfs, but plugin utbot doesn't use this flag
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 27);

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
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 90);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, Many_Pointers_In_Struct) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 100);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, Complex_Struct) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 104);

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

    TEST_F(Syntax_Test, Pass_Pointer_To_Struct_With_Pointer) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 111);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, Check_Error_Tests_Have_Fail_Assertion) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 111);

        ASSERT_TRUE(status.ok()) << status.error_message();

        for (const auto &[source_file_path, tests] : testGen.tests) {
            EXPECT_THAT(tests.code,
                        ::testing::HasSubstr(
                            "FAIL() << \"Unreachable point or the function was supposed to fail"));
        }
    }

    TEST_F(Syntax_Test, Pass_Pointer_To_Const_Struct_With_Pointer) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 115);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, Check_Lazy_Pointers_In_Struct) {
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 78);

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
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 78);

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
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 86);

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
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 94);

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
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 6);

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
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 15);

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
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 24);

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
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 36);

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
        auto [testGen, status] = createTestForFunction(structs_with_pointers_c, 78);
        const std::string code = testGen.tests.begin()->second.code;
        const std::string beginRegressionRegion = "#pragma region " + Tests::DEFAULT_SUITE_NAME + NL;
        const std::string endRegion = std::string("#pragma endregion") + NL;
        const std::string beginErrorRegion = "#pragma region " + Tests::ERROR_SUITE_NAME + NL;
        ASSERT_TRUE(code.find(beginRegressionRegion) != std::string::npos) << "No regression begin region";
        ASSERT_TRUE(code.find(endRegion) != std::string::npos) << "No regression end region";
        ASSERT_TRUE(code.find(beginErrorRegion) != std::string::npos) << "No error begin region";
    }

    TEST_F(Syntax_Test, Function_Pointers_StructFieldParam) {
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 48);

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
        auto [testGen, status] = createTestForFunction(functions_as_params_c, 69);

        ASSERT_TRUE(status.ok()) << status.error_message();
        EXPECT_EQ(1, testUtils::getNumberOfTests(testGen.tests));
    }

    TEST_F(Syntax_Test, Variadic_Test) {
        auto [testGen, status] = createTestForFunction(variadic_c, 4);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(variadic_c).methods.begin().value().testCases, 3);
    }

    TEST_F(Syntax_Test, Struct_with_Char_Pointer) {
        auto [testGen, status] = createTestForFunction(types_c, 58);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Recursive_Struct) {
        auto [testGen, status] = createTestForFunction(types_c, 62);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Struct_With_Const_Pointer_Return) {
        auto [testGen, status] = createTestForFunction(types_c, 83);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Struct_With_Const_Pointer_Return_Pointer) {
        auto [testGen, status] = createTestForFunction(types_c, 103);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Struct_Const_Pointer_Param) {
        auto [testGen, status] = createTestForFunction(types_c, 109);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Multi_Array_1) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 23);

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
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 64);

        ASSERT_TRUE(status.ok()) << status.error_message();
        testUtils::checkMinNumberOfTests(testGen.tests.at(multi_arrays_c).methods.begin().value().testCases, 2);
    }


    TEST_F(Syntax_Test, Struct_With_Multi_Array) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 76);

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
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 116);

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
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 131);

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
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 150);

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
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 170);

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
        auto [testGen, status] = createTestForFunction(floats_special_c, 6);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(floats_special_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[](const tests::Tests::MethodTestCase &testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "NAN";
                        }}));
    }

    TEST_F(Syntax_Test, Floats_Special_Values_Nan) {
        auto [testGen, status] = createTestForFunction(floats_special_c, 14);


        ASSERT_TRUE(status.ok()) << status.error_message();

        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::C);
        const auto &tests = testGen.tests.at(floats_special_c)
                                .methods.begin().value().testCases;
        checkTestCasePredicates(
            tests, std::vector<TestCasePredicate>(
                       { [](const tests::Tests::MethodTestCase &testCase) {
                            return testCase.paramValues[0].view->getEntryValue(nullptr) == "NAN";
                         },
                         [](const tests::Tests::MethodTestCase &testCase) {
                             return testCase.paramValues[0].view->getEntryValue(nullptr) == "0.000000e+00L";
                         } }));
    }

    TEST_F(Syntax_Test, Floats_Special_Values_Inf) {
        auto [testGen, status] = createTestForFunction(floats_special_c, 23);


        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(floats_special_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                    { [](const tests::Tests::MethodTestCase &testCase) {
                        return testCase.paramValues[0].view->getEntryValue(nullptr) == "INFINITY";
                    } }));
    }

    TEST_F(Syntax_Test, Accept_Const_Int_Const_Pointer_Const_Pointer) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 182);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Accept_Const_Int_Const_Pointer_Pointer) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 186);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Accept_Const_Int_Pointer_Pointer) {
        auto [testGen, status] = createTestForFunction(multi_arrays_c, 190);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests, 1);
    }


    TEST_F(Syntax_Test, Supported_2d_Pointer) {
        auto [testGen, status] = createTestForFunction(types_c, 68);

        ASSERT_TRUE(status.ok()) << status.error_message();

        int numberOfTests = testUtils::getNumberOfTests(testGen.tests);
        EXPECT_EQ(2, numberOfTests);
    }

    TEST_F(Syntax_Test, Supported_Void_Pointer) {
        auto [testGen, status] = createTestForFunction(types_c, 73);

        ASSERT_TRUE(status.ok()) << status.error_message();

        int numberOfTests = testUtils::getNumberOfTests(testGen.tests);
        EXPECT_EQ(1, numberOfTests);
    }

    TEST_F(Syntax_Test, Support_Struct_with_Union1) {
        auto [testGen, status] = createTestForFunction(struct_with_union_c, 3);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(struct_with_union_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) != 0 &&
                                   testCase.returnValue.view->getEntryValue(nullptr) == "{{17}, {{-1414812880}}, -108}";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) == 0 &&
                                    testCase.returnValue.view->getEntryValue(nullptr) == "{{-1414812831}, {{101}}, 155}";
                         }
                        })
        );
    }

    TEST_F(Syntax_Test, Support_Struct_with_Union2) {
        auto [testGen, status] = createTestForFunction(struct_with_union_c, 17);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(struct_with_union_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) +
                                   stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) < 0 &&
                                   testCase.returnValue.view->getEntryValue(nullptr) == "{{{-2.530171e-98}}}";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) +
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) >= 0 &&
                                    stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) +
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) <= 16 &&
                                    testCase.returnValue.view->getEntryValue(nullptr) == "{{{1.410000e+00}}}";
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) +
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) > 16 &&
                                    testCase.returnValue.view->getEntryValue(nullptr) == "{{{-2.530171e-98}}}";
                         }
                        })
        );
    }

    TEST_F(Syntax_Test, Support_Struct_with_Union3) {
        auto [testGen, status] = createTestForFunction(struct_with_union_c, 29);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(struct_with_union_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {[] (const tests::Tests::MethodTestCase& testCase) {
                            return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) <
                                   stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                            testCase.returnValue.view->getEntryValue(nullptr) == "{{{'\\x99', -2.530171e-98}}}";
                        },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) ==
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                                    StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                            "{from_bytes<StructWithStructInUnion::DeepUnion>({");;
                         },
                         [] (const tests::Tests::MethodTestCase& testCase) {
                             return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >
                                    stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                             testCase.returnValue.view->getEntryValue(nullptr) == "{{{'\\0', -2.530171e-98}}}";
                         }
                        })
        );
    }

    TEST_F(Syntax_Test, Support_Struct_with_Union_Of_Unnamed_Type) {
        auto [testGen, status] = createTestForFunction(struct_with_union_c, 42);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(struct_with_union_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {[] (const tests::Tests::MethodTestCase& testCase) {
                     return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) <
                            stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                            testCase.returnValue.view->getEntryValue(nullptr) == "{{{'\\x99', -2.530171e-98}}}";
                 },
                  [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) ==
                             stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                             StringUtils::startsWith(testCase.returnValue.view->getEntryValue(nullptr),
                                                     "{from_bytes<StructWithUnionOfUnnamedType_un>({");
                  },
                  [] (const tests::Tests::MethodTestCase& testCase) {
                      return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >
                             stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) &&
                             testCase.returnValue.view->getEntryValue(nullptr) == "{{{'\\0', -2.530171e-98}}}";
                  }
                })
        );
    }

    TEST_F(Syntax_Test, length_of_linked_list3) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 3);

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
        auto [testGen, status] = createTestForFunction(linked_list_c, 19);

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
        auto [testGen, status] = createTestForFunction(linked_list_c, 32);

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
        auto [testGen, status] = createTestForFunction(linked_list_c, 45);

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
        auto [testGen, status] = createTestForFunction(linked_list_c, 58);

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

    TEST_F(Syntax_Test, DISABLED_len_bound) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 92);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(linked_list_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                    return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                }
            })
        );
    }

    TEST_F(Syntax_Test, sort_list) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 104, 90);

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
        auto [testGen, status] = createTestForFunction(linked_list_c, 135, 90);

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
        auto [testGen, status] = createTestForFunction(array_sort_c, 5);

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
        auto [testGen, status] = createTestForFunction(array_sort_c, 33);

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
        auto [testGen, status] = createTestForFunction(stubs_c, 3);

        ASSERT_TRUE(status.ok()) << status.error_message();

        EXPECT_EQ(2, testUtils::getNumberOfTests(testGen.tests));
    }

    TEST_F(Syntax_Test, vowel_consonant) {
        auto [testGen, status] = createTestForFunction(stubs_c, 12);

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
        auto [testGen, status] = createTestForFunction(tree_c, 3);

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
        auto [testGen, status] = createTestForFunction(types_3_c, 15);

        // bug #317 fixed
        ASSERT_TRUE(status.ok());
        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 2);
    }

    TEST_F(Syntax_Test, UnnamedTypeStructField) {
        auto [testGen, status] = createTestForFunction(types_3_c, 33);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 2);
    }

    TEST_F(Syntax_Test, AnonymousUnionField) {
        auto [testGen, status] = createTestForFunction(types_3_c, 48);

        ASSERT_TRUE(status.ok()) << status.error_message();
    }

    TEST_F(Syntax_Test, AnonymousStructField) {
        auto [testGen, status] = createTestForFunction(types_3_c, 65);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 2);
    }

    TEST_F(Syntax_Test, Vector_Sum) {
        auto [testGen, status] = createTestForFunction(types_3_c, 76);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Vector_Create) {
        auto [testGen, status] = createTestForFunction(types_3_c, 91);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Accept_Incomplete) {
        auto [testGen, status] = createTestForFunction(types_3_c, 101);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Return_Incomplete) {
        auto [testGen, status] = createTestForFunction(types_3_c, 105);

        ASSERT_TRUE(status.error_code() == grpc::FAILED_PRECONDITION) << status.error_message();
    }

    TEST_F(Syntax_Test, Pass_Forward_Decl) {
        auto [testGen, status] = createTestForFunction(types_3_c, 112);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Duplicate_Struct) {
        auto [testGen, status] = createTestForFunction(types_3_c, 128);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(types_3_c).methods.begin().value().testCases, 3);
    }

    TEST_F(Syntax_Test, Global_Unnamed_Variable) {
        auto [testGen, status] = createTestForFunction(types_3_c, 145);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(types_3_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>({ [](const tests::Tests::MethodTestCase &testCase) {
              EXPECT_TRUE(testCase.globalPostValues.empty());
              return testCase.returnValue.view->getEntryValue(nullptr) == "-1";
            } }),
            "check_option");
    }

    TEST_F(Syntax_Test, Simple_parameter_cpp) {
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 4);

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
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 11);

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
        auto[testGen, status] = createTestForFunction(different_parameters_cpp, 19);

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
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 25);

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
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 39);

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
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 46);

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
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 53);

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
        auto [testGen, status] = createTestForFunction(different_parameters_cpp, 60);

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
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 16);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Operator_plus_eq_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 24);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Operator_plus_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 30);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Change_class_by_ref_cpp) {
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 34);

        ASSERT_TRUE(status.ok()) << status.error_message();
        printer::TestsPrinter testsPrinter(testGen.projectContext, nullptr, utbot::Language::CXX);
        const auto &tests = testGen.tests.at(simple_class_cpp)
                                .methods.begin().value().testCases;
        testUtils::checkRegexp(tests[0].paramValues[0].view->getEntryValue(&testsPrinter),
                               "[{]"
                               "\n    /[*][.]x = [*]/.+,"
                               "\n    /[*][.]y = [*]/.+"
                               "\n[}]");

        testUtils::checkMinNumberOfTests(testGen.tests.at(simple_class_cpp).methods.begin().value().testCases, 5);

        checkTestCasePredicates(
              tests,
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
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 50);

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
        auto [testGen, status] = createTestForFunction(simple_class_cpp, 60);

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
        auto[testGen, status] = createTestForFunction(inner_unnamed_c, 4);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(inner_unnamed_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    std::stringstream ss;
                    EXPECT_EQ(testCase.paramValues.front().view->getEntryValue(nullptr).size(), 3);
                    ss << "{{"
                       // "'x'"[1] => int('x')
                       << int(testCase.paramValues.front().view->getEntryValue(nullptr)[1])
                       << "}, "
                       << int(testCase.paramValues.front().view->getEntryValue(nullptr)[1])
                       << "}";
                    return testCase.returnValue.view->getEntryValue(nullptr) == ss.str();
                }}));
    }

    TEST_F(Syntax_Test, Inner_unnamed_union_parameter) {
        auto[testGen, status] = createTestForFunction(inner_unnamed_c, 11);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(inner_unnamed_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    return "{{0}, 0}" == testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "{{42}, 42}" == testCase.returnValue.view->getEntryValue(nullptr);
                }, [](const tests::Tests::MethodTestCase &testCase) {
                    return "{{0}, 0}" != testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "{{24}, 24}" == testCase.returnValue.view->getEntryValue(nullptr);

                }}));
    }

    TEST_F(Syntax_Test, Inner_unnamed_struct_return) {
        auto[testGen, status] = createTestForFunction(inner_unnamed_c, 23);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(inner_unnamed_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                    std::stringstream ss;
                    EXPECT_EQ(testCase.paramValues.front().view->getEntryValue(nullptr).size(), 3);
                    ss << "{{'"
                       << char(testCase.paramValues.front().view->getEntryValue(nullptr)[1])
                       << "', "
                       << int(testCase.paramValues.front().view->getEntryValue(nullptr)[1])
                       << "}}";
                    return testCase.returnValue.view->getEntryValue(nullptr) == ss.str();
                }}));
    }


    TEST_F(Syntax_Test, Inner_unnamed_struct_parameter) {
        auto[testGen, status] = createTestForFunction(inner_unnamed_c, 29);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
                testGen.tests.at(inner_unnamed_c).methods.begin().value().testCases,
                std::vector<TestCasePredicate>({[](const tests::Tests::MethodTestCase &testCase) {
                     return "{{'\\0', 0}}" ==
                           testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "{{'*', 42}}" ==
                           testCase.returnValue.view->getEntryValue(nullptr);
                }, [](const tests::Tests::MethodTestCase &testCase) {
                    return "{{'\\0', 0}}" !=
                           testCase.paramValues.front().view->getEntryValue(nullptr) &&
                           "{{'\\x18', 24}}" ==
                           testCase.returnValue.view->getEntryValue(nullptr);

                }}));
    }

    TEST_F(Syntax_Test, Typedef_to_pointer_array) {
        auto[testGen, status] = createTestForFunction(pointer_parameters_c, 39);

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

    TEST_F(Syntax_Test, Default_constructor_cpp) {
        auto [testGen, status] = createTestForFunction(constructors_cpp, 59);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(constructors_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Constructor_with_parameters_cpp) {
        auto [testGen, status] = createTestForFunction(constructors_cpp, 86);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(constructors_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Copy_constructor_cpp) {
        auto [testGen, status] = createTestForFunction(constructors_cpp, 37);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(constructors_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Move_constructor_cpp) {
        auto [testGen, status] = createTestForFunction(constructors_cpp, 67);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(constructors_cpp).methods.begin().value().testCases, 1);
    }

    TEST_F(Syntax_Test, Constructor_with_pointers_cpp) {
        auto [testGen, status] = createTestForFunction(constructors_cpp, 21);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(constructors_cpp).methods.begin().value().testCases, 2);
    }

    TEST_F(Syntax_Test, Constructor_with_if_stmt_cpp) {
        auto [testGen, status] = createTestForFunction(constructors_cpp, 9);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(
                testGen.tests.at(constructors_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
                testGen.tests.at(constructors_cpp).methods.begin().value().testCases,
                std::vector<TestCasePredicate>(
                        {
                                [](const tests::Tests::MethodTestCase &testCase) {
                                    return "false" == testCase.paramValues.front().view->getEntryValue(nullptr);
                                },
                                [](const tests::Tests::MethodTestCase &testCase) {
                                    return "true" == testCase.paramValues.front().view->getEntryValue(nullptr);
                                }
                        }));
    }

    TEST_F(Syntax_Test, void_ptr) {
        auto [testGen, status] = createTestForFunction(pointer_parameters_c, 45);

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
        auto [testGen, status] = createTestForFunction(linked_list_c, 166);

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

    TEST_F(Syntax_Test, content_of_void_ptr) {
        auto [testGen, status] = createTestForFunction(linked_list_c, 182);

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

    TEST_F(Syntax_Test, example_namespace_cpp) {
        auto [testGen, status] = createTestForFunction(namespace_cpp, 3);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(namespace_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return testCase.returnValue.view->getSubViews()[2]->getEntryValue(nullptr) == "{17}" &&
                             stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == 5;
                    },
                     [] (const tests::Tests::MethodTestCase& testCase) {
                       return testCase.returnValue.view->getSubViews()[2]->getEntryValue(nullptr) == "{101}" &&
                              stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == -1;
                     },
                     [] (const tests::Tests::MethodTestCase& testCase) {
                       return testCase.returnValue.view->getSubViews()[2]->getEntryValue(nullptr) == "{-1414812822}" &&
                              stoi(testCase.returnValue.view->getSubViews()[0]->getEntryValue(nullptr)) == 10;
                     }
                })
        );
    }

    TEST_F(Syntax_Test, struct_with_union_as_return_type_cpp) {
        auto [testGen, status] = createTestForFunction(namespace_cpp, 24);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(namespace_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return testCase.returnValue.view->getEntryValue(nullptr) == "{{17}, {{-1414812880}}, -108}";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "{{-1414812831}, {{101}}, 155}";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, multi_union_cpp) {
        auto [testGen, status] = createTestForFunction(namespace_cpp, 38);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(namespace_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return testCase.returnValue.view->getEntryValue(nullptr) == "{{{5}}, {6}}";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                      return testCase.returnValue.view->getEntryValue(nullptr) == "{{{10}}, {9}}";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, multiple_rvalue_params_cpp) {
        auto [testGen, status] = createTestForFunction(rvalue_reference_cpp, 9);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases, 2);

        checkTestCasePredicates(
            testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) >
                               stoi(testCase.paramValues[1].view->getEntryValue(nullptr));
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) <=
                               stoi(testCase.paramValues[1].view->getEntryValue(nullptr));
                    }
                })
        );

        checkTestCasePredicates(
            testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return 2 * stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) ==
                               stoi(testCase.returnValue.view->getEntryValue(nullptr));
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return 2 * stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) ==
                               stoi(testCase.returnValue.view->getEntryValue(nullptr));
                    }
                })
        );

    }

    TEST_F(Syntax_Test, const_rvalue_reference_cpp) {
        auto [testGen, status] = createTestForFunction(rvalue_reference_cpp, 17);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases, 3);

        checkTestCasePredicates(
            testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases,
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
                    }
                })
        );

        checkTestCasePredicates(
            testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues.front().view->getEntryValue(nullptr)) % 3 == 0;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues.front().view->getEntryValue(nullptr)) % 3 == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        auto paramValue = stoi(testCase.paramValues.front().view->getEntryValue(nullptr));
                        return paramValue % 3 != 0 && paramValue % 3 != 1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, return_and_get_params_cpp) {
        auto [testGen, status] = createTestForFunction(rvalue_reference_cpp, 28);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases, 3);

        checkTestCasePredicates(
            testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) % 5 == 0;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) % 5 == 0;
                    }
                })
        );

        checkTestCasePredicates(
            testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) ==
                               stoi(testCase.returnValue.view->getEntryValue(nullptr));
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues[1].view->getEntryValue(nullptr)) ==
                               stoi(testCase.returnValue.view->getEntryValue(nullptr));
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.paramValues[0].view->getEntryValue(nullptr)) + stoi(testCase.paramValues[1].view->getEntryValue(nullptr))==
                               stoi(testCase.returnValue.view->getEntryValue(nullptr));
                    }
                })
        );
    }

    TEST_F(Syntax_Test, rvalue_struct_param_cpp) {
        auto [testGen, status] = createTestForFunction(rvalue_reference_cpp, 38);

        ASSERT_TRUE(status.ok()) << status.error_message();

        testUtils::checkMinNumberOfTests(testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases, 4);

        checkTestCasePredicates(
            testGen.tests.at(rvalue_reference_cpp).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, unsupported_clases_cpp) {
        std::vector<size_t> lines = {4, 8, 12, 16};
        for (const auto &line: lines) {
            auto [testGen, status] = createTestForFunction(unsupported_class_cpp, line);
            ASSERT_FALSE(status.ok());
        }
    }

    TEST_F(Syntax_Test, simple_getc) {
        auto [testGen, status] = createTestForFunction(input_output_c, 4);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
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
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 4;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 5;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 6;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 7;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 8;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 9;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_fgetc) {
        auto [testGen, status] = createTestForFunction(input_output_c, 30);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
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
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_fread) {
        auto [testGen, status] = createTestForFunction(input_output_c, 53);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_fgets) {
        auto [testGen, status] = createTestForFunction(input_output_c, 73);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_getchar) {
        auto [testGen, status] = createTestForFunction(input_output_c, 82);

        ASSERT_TRUE(status.ok()) << status.error_message();

        for (const auto &testCase: testGen.tests.at(input_output_c).methods.begin().value().testCases) {
            ASSERT_TRUE(stoi(testCase.returnValue.view->getEntryValue(nullptr)) != -1);
        }

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
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
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_gets) {
        auto [testGen, status] = createTestForFunction(input_output_c, 99);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_putc) {
        auto [testGen, status] = createTestForFunction(input_output_c, 108);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'0'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'1'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'2'";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_fputc) {
        auto [testGen, status] = createTestForFunction(input_output_c, 121);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'<'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'>'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'='";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_fwrite) {
        auto [testGen, status] = createTestForFunction(input_output_c, 134);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'P'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'N'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'Z'";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_fputs) {
        auto [testGen, status] = createTestForFunction(input_output_c, 150);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'V'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'C'";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_putchar) {
        auto [testGen, status] = createTestForFunction(input_output_c, 162);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'>'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'<'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'='";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, simple_puts) {
        auto [testGen, status] = createTestForFunction(input_output_c, 175);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(input_output_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'V'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'C'";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, file_fgetc) {
        auto [testGen, status] = createTestForFunction(file_c, 5);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(file_c).methods.begin().value().testCases,
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
                    }
                })
        );
    }

    TEST_F(Syntax_Test, file_fgets) {
        auto [testGen, status] = createTestForFunction(file_c, 29);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(file_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, file_fputc) {
        auto [testGen, status] = createTestForFunction(file_c, 38);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(file_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'<'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'>'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'='";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, file_fputs) {
        auto [testGen, status] = createTestForFunction(file_c, 51);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(file_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'V'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'C'";
                    }
                })
        );
    }

    TEST_F(Syntax_Test, sum_two_from_file) {
        auto [testGen, status] = createTestForFunction(file_c, 63);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(file_c).methods.begin().value().testCases,
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

    TEST_F(Syntax_Test, file_fread) {
        auto [testGen, status] = createTestForFunction(file_c, 76);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(file_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 0;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                    }
                })
        );
    }

    TEST_F(Syntax_Test, file_fwrite) {
        auto [testGen, status] = createTestForFunction(file_c, 96);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(file_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'P'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'N'";
                    },
                    [] (const tests::Tests::MethodTestCase& testCase) {
                        return testCase.returnValue.view->getEntryValue(nullptr) == "'Z'";
                    }
                })
        );
    }

    template<typename T>
    bool checkBitfieldFit(const std::shared_ptr<tests::AbstractValueView> &fieldView, size_t size) {
        T val = StringUtils::stot<T>(fieldView->getEntryValue(nullptr));
        T minVal, maxVal, one = 1;
        if constexpr (std::is_signed_v<T>) {
            minVal = -(one << (size - 1));
            maxVal = -(minVal + 1);
        } else {
            minVal = 0;
            maxVal = (one << size) - 1;
        }
        return val >= minVal && val <= maxVal;
    }

    TEST_F(Syntax_Test, bitfields_check_simple_signed_str) {
        auto [testGen, status] = createTestForFunction(bitfields_c, 26);

        ASSERT_TRUE(status.ok()) << status.error_message();

        for (const auto &testCase: testGen.tests.at(bitfields_c).methods.begin().value().testCases) {
            ASSERT_TRUE(!testCase.isError());
        }

        checkTestCasePredicates(
            testGen.tests.at(bitfields_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return checkBitfieldFit<int>(subViews[0], 24) &&
                               checkBitfieldFit<int>(subViews[1], 1) &&
                               checkBitfieldFit<int>(subViews[2], 2) &&
                               checkBitfieldFit<int>(subViews[3], 5) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "1";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return checkBitfieldFit<int>(subViews[0], 24) &&
                               checkBitfieldFit<int>(subViews[1], 1) &&
                               checkBitfieldFit<int>(subViews[2], 2) &&
                               checkBitfieldFit<int>(subViews[3], 5) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "-1";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return checkBitfieldFit<int>(subViews[0], 24) &&
                               checkBitfieldFit<int>(subViews[1], 1) &&
                               checkBitfieldFit<int>(subViews[2], 2) &&
                               checkBitfieldFit<int>(subViews[3], 5) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "0";
                }
            })
        );
    }

    TEST_F(Syntax_Test, bitfields_check_fields_bounds) {
        auto [testGen, status] = createTestForFunction(bitfields_c, 106);

        ASSERT_TRUE(status.ok()) << status.error_message();

        for (const auto &testCase: testGen.tests.at(bitfields_c).methods.begin().value().testCases) {
            ASSERT_TRUE(!testCase.isError());
        }

        checkTestCasePredicates(
            testGen.tests.at(bitfields_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                {
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return checkBitfieldFit<unsigned>(subViews[0], 7) &&
                               checkBitfieldFit<long long>(subViews[1],
                                                           SizeUtils::bytesToBits(
                                                                   sizeof(long long))) &&
                               checkBitfieldFit<signed>(subViews[2], 17) &&
                               checkBitfieldFit<bool>(subViews[3], 1) &&
                               checkBitfieldFit<int>(subViews[4], 22) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "1";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return checkBitfieldFit<unsigned>(subViews[0], 7) &&
                               checkBitfieldFit<long long>(subViews[1],
                                                           SizeUtils::bytesToBits(
                                                                   sizeof(long long))) &&
                               checkBitfieldFit<signed>(subViews[2], 17) &&
                               checkBitfieldFit<bool>(subViews[3], 1) &&
                               checkBitfieldFit<int>(subViews[4], 22) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "2";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return checkBitfieldFit<unsigned>(subViews[0], 7) &&
                               checkBitfieldFit<long long>(subViews[1],
                                                           SizeUtils::bytesToBits(
                                                                   sizeof(long long))) &&
                               checkBitfieldFit<signed>(subViews[2], 17) &&
                               checkBitfieldFit<bool>(subViews[3], 1) &&
                               checkBitfieldFit<int>(subViews[4], 22) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "3";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return checkBitfieldFit<unsigned>(subViews[0], 7) &&
                               checkBitfieldFit<long long>(subViews[1],
                                                           SizeUtils::bytesToBits(
                                                                   sizeof(long long))) &&
                               checkBitfieldFit<signed>(subViews[2], 17) &&
                               checkBitfieldFit<bool>(subViews[3], 1) &&
                               checkBitfieldFit<int>(subViews[4], 22) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "4";
                }
            })
        );
    }

    TEST_F(Syntax_Test, bitfields_check_unnamed) {
        auto [testGen, status] = createTestForFunction(bitfields_c, 99);

        ASSERT_TRUE(status.ok()) << status.error_message();

        for (const auto &testCase: testGen.tests.at(bitfields_c).methods.begin().value().testCases) {
            ASSERT_TRUE(!testCase.isError());
        }

        checkTestCasePredicates(
        testGen.tests.at(bitfields_c).methods.begin().value().testCases,
        std::vector<TestCasePredicate>(
                {
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return subViews.size() == 3 &&
                               checkBitfieldFit<unsigned>(subViews[0], 7) &&
                               checkBitfieldFit<unsigned>(subViews[1], 6) &&
                               checkBitfieldFit<unsigned>(subViews[2], 15) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "0";
                    },
                    [](const tests::Tests::MethodTestCase &testCase) {
                        auto &subViews = testCase.paramValues.front().view->getSubViews();
                        return subViews.size() == 3 &&
                               checkBitfieldFit<unsigned>(subViews[0], 7) &&
                               checkBitfieldFit<unsigned>(subViews[1], 6) &&
                               checkBitfieldFit<unsigned>(subViews[2], 15) &&
                               testCase.returnValue.view->getEntryValue(nullptr) == "13";
                }
            })
        );
    }

    TEST_F(Syntax_Test, hard_list_and_pointers) {
        auto [testGen, status] = createTestForFunction(hard_linked_list_c, 5);

        ASSERT_TRUE(status.ok()) << status.error_message();

        checkTestCasePredicates(
            testGen.tests.at(hard_linked_list_c).methods.begin().value().testCases,
            std::vector<TestCasePredicate>(
                { [](const tests::Tests::MethodTestCase &testCase) {
                     return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -1;
                 },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 1;
                  },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -2;
                  },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 2;
                  },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == -3;
                  },
                  [](const tests::Tests::MethodTestCase &testCase) {
                      return stoi(testCase.returnValue.view->getEntryValue(nullptr)) == 3;
                  } }));
    }
}
