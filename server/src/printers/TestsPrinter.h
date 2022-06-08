/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TESTSPRINTER_H
#define UNITTESTBOT_TESTSPRINTER_H

#include "Printer.h"
#include "Tests.h"
#include "building/BuildDatabase.h"
#include "stubs/Stubs.h"
#include "types/Types.h"
#include "utils/PrinterUtils.h"
#include "utils/path/FileSystemPath.h"
#include "utils/ErrorMode.h"

#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using tests::Tests;

namespace printer {
    class TestsPrinter : public Printer {
    public:
        explicit TestsPrinter(const types::TypesHandler *typesHandler, utbot::Language srcLanguage);

        utbot::Language getLanguage() const override;

        void genCode(Tests::MethodDescription &methodDescription,
                     const std::optional<LineInfo::PredicateInfo>& predicateInfo = {},
                     bool verbose = false,
                     ErrorMode::ErrorMode errorMode = ErrorMode::ErrorMode::FAILING);

        void joinToFinalCode(Tests &tests, const fs::path &generatedHeaderPath);

        static bool needsMathHeader(const Tests &tests);

        void genHeaders(Tests &tests, const fs::path &generatedHeaderPath);

        void genParametrizedTestCase(const tests::Tests::MethodDescription &methodDescription,
                                     const Tests::MethodTestCase &testCase,
                                     const std::optional<LineInfo::PredicateInfo>& predicateInfo,
                                     ErrorMode::ErrorMode errorMode);

        void genVerboseTestCase(const tests::Tests::MethodDescription &methodDescription,
                                const Tests::MethodTestCase &testCase,
                                const std::optional<LineInfo::PredicateInfo> &predicateInfo,
                                ErrorMode::ErrorMode errorMode);

        void testHeader(const std::string &scopeName,
                        const tests::Tests::MethodDescription &methodDescription,
                        int testNum);

        void redirectStdin(const tests::Tests::MethodDescription &methodDescription,
                           const Tests::MethodTestCase &testCase,
                           bool verbose);

        void verboseParameter(const tests::Tests::MethodDescription &method,
                              const Tests::MethodParam &param,
                              const Tests::TestCaseParamValue &value,
                              bool needDeclaration);

        void printClassObject(const tests::Tests::MethodDescription &methodDescription,
                              const Tests::MethodTestCase &testCase);

        void verboseParameters(const tests::Tests::MethodDescription &methodDescription,
                               const Tests::MethodTestCase &testCase);

        void printFunctionParameters(const tests::Tests::MethodDescription &methodDescription,
                                     const Tests::MethodTestCase &testCase,
                                     bool all);

        void verboseOutputVariable(const tests::Tests::MethodDescription &methodDescription,
                                   const Tests::MethodTestCase &testCase);

        void verboseFunctionCall(const tests::Tests::MethodDescription &methodDescription,
                                 const Tests::MethodTestCase &testCase,
                                 ErrorMode::ErrorMode errorMode);

        void verboseAsserts(const tests::Tests::MethodDescription &methodDescription,
                            const Tests::MethodTestCase &testCase,
                            const std::optional<LineInfo::PredicateInfo>& predicateInfo);

        void classAsserts(const Tests::MethodDescription &methodDescription,
                                        const Tests::MethodTestCase &testCase);

        void changeableParamsAsserts(const Tests::MethodDescription &methodDescription,
                            const Tests::MethodTestCase &testCase);

        void globalParamsAsserts(const Tests::MethodDescription &methodDescription,
                            const Tests::MethodTestCase &testCase);

        void parametrizedArrayParameters(const tests::Tests::MethodDescription &methodDescription,
                                         const Tests::MethodTestCase &testCase);

        void parametrizedAsserts(const tests::Tests::MethodDescription &methodDescription,
                                 const Tests::MethodTestCase &testCase,
                                 const std::optional<LineInfo::PredicateInfo>& predicateInfo,
                                 ErrorMode::ErrorMode errorMode);

        static std::vector<std::string>
        methodParametersListParametrized(const tests::Tests::MethodDescription &methodDescription,
                                         const Tests::MethodTestCase &testCase);

        static std::vector<std::string>
        methodParametersListVerbose(const tests::Tests::MethodDescription &methodDescription,
                                    const Tests::MethodTestCase &testCase);


        std::string constrVisitorFunctionCall(const tests::Tests::MethodDescription &methodDescription,
                                              const Tests::MethodTestCase &testCase,
                                              bool verboseMode,
                                              ErrorMode::ErrorMode errorMode);

        struct FunctionSignature {
            std::string name;
            std::vector<std::string> args;
        };

    private:
        types::TypesHandler const *typesHandler;

        void
        parametrizedInitializeGlobalVariables(const Tests::MethodDescription &methodDescription,
                                              const Tests::MethodTestCase &testCase);

        void
        parametrizedInitializeSymbolicStubs(const Tests::MethodDescription &methodDescription,
                                            const Tests::MethodTestCase &testCase);

        void printLazyVariables(const Tests::MethodDescription &methodDescription,
                                const Tests::MethodTestCase &testCase,
                                bool verbose);

        void printLazyVariables(const std::vector<Tests::MethodParam> &lazyParams,
                                const std::vector<Tests::TestCaseParamValue> &lazyValues);

        void printLazyReferences(const Tests::MethodDescription &methodDescription,
                                 const Tests::MethodTestCase &testCase,
                                 bool verbose);

        void printStubVariables(const Tests::MethodDescription &methodDescription,
                                const Tests::MethodTestCase &testCase);

        static Tests::MethodParam getValueParam(const Tests::MethodParam &param);

        void genCodeBySuiteName(const std::string &targetSuiteName,
                                Tests::MethodDescription &methodDescription,
                                const std::optional<LineInfo::PredicateInfo>& predicateInfo,
                                bool verbose,
                                int &testNum,
                                ErrorMode::ErrorMode errorMode);

        std::uint32_t printSuiteAndReturnMethodsCount(const std::string &suiteName, const Tests::MethodsMap &methods);
    };
}
#endif // UNITTESTBOT_TESTSPRINTER_H
