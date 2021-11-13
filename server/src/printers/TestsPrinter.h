/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TESTSPRINTER_H
#define UNITTESTBOT_TESTSPRINTER_H

#include "Printer.h"
#include "StmtBordersFinder.h"
#include "Tests.h"
#include "building/BuildDatabase.h"
#include "stubs/Stubs.h"
#include "types/Types.h"
#include "utils/PrinterUtils.h"

#include "loguru.h"

#include <cstdio>
#include "utils/path/FileSystemPath.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::vector;
using std::unordered_map;
using std::cout;
using std::endl;

using tests::Tests;

namespace printer {
    class TestsPrinter : public Printer {
    public:
        explicit TestsPrinter(const types::TypesHandler *typesHandler, utbot::Language srcLanguage);

        utbot::Language getLanguage() const override;

        void genCode(Tests::MethodDescription &methodDescription,
                     const std::optional<LineInfo::PredicateInfo>& predicateInfo = {},
                     bool verbose = false);

        void joinToFinalCode(Tests &tests, const fs::path &generatedHeaderPath);

        static bool needsMathHeader(const Tests &tests);

        void genHeaders(Tests &tests, const fs::path &generatedHeaderPath);

        void genParametrizedTestCase(const tests::Tests::MethodDescription &methodDescription,
                                     const Tests::MethodTestCase &testCase,
                                     const std::optional<LineInfo::PredicateInfo>& predicateInfo);

        void genVerboseTestCase(const tests::Tests::MethodDescription &methodDescription,
                                const Tests::MethodTestCase &testCase,
                                std::optional<LineInfo::PredicateInfo> predicateInfo);

        void testHeader(const string &scopeName,
                        const tests::Tests::MethodDescription &methodDescription,
                        int testNum);

        void redirectStdin(const tests::Tests::MethodDescription &methodDescription,
                           const Tests::MethodTestCase &testCase,
                           bool verbose);

        void verboseParameter(const tests::Tests::MethodDescription &method,
                              const Tests::MethodParam &param,
                              const Tests::TestCaseParamValue &value,
                              bool needDeclaration);

        void printClassObject(const tests::Tests::MethodDescription &methodDescription);

        void verboseParameters(const tests::Tests::MethodDescription &methodDescription,
                               const Tests::MethodTestCase &testCase);

        void printFunctionParameters(const tests::Tests::MethodDescription &methodDescription,
                                     const Tests::MethodTestCase &testCase,
                                     bool onlyLValue = false);

        void verboseOutputVariable(const tests::Tests::MethodDescription &methodDescription,
                                   const Tests::MethodTestCase &testCase);

        void verboseFunctionCall(const tests::Tests::MethodDescription &methodDescription,
                                 const Tests::MethodTestCase &testCase);

        void verboseAsserts(const tests::Tests::MethodDescription &methodDescription,
                            const Tests::MethodTestCase &testCase,
                            std::optional<LineInfo::PredicateInfo> predicateInfo);

        void changeableParamsAsserts(const Tests::MethodDescription &methodDescription,
                            const Tests::MethodTestCase &testCase);

        void globalParamsAsserts(const Tests::MethodDescription &methodDescription,
                            const Tests::MethodTestCase &testCase);

        void parametrizedArrayParameters(const tests::Tests::MethodDescription &methodDescription,
                                         const Tests::MethodTestCase &testCase);

        void parametrizedAsserts(const tests::Tests::MethodDescription &methodDescription,
                                 const Tests::MethodTestCase &testCase,
                                 const std::optional<LineInfo::PredicateInfo>& predicateInfo);

        static std::vector<string>
        methodParametersListParametrized(const tests::Tests::MethodDescription &methodDescription,
                             const Tests::MethodTestCase &testCase);

        static std::vector<string>
        methodParametersListVerbose(const tests::Tests::MethodDescription &methodDescription,
                                    const Tests::MethodTestCase &testCase);


        string constrVisitorFunctionCall(const tests::Tests::MethodDescription &methodDescription,
                                         const Tests::MethodTestCase &testCase,
                                         bool verboseMode);

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
                                const Tests::MethodTestCase &testCase);

        void printLazyReferences(const Tests::MethodDescription &methodDescription,
                                 const Tests::MethodTestCase &testCase);

        void printStubVariables(const Tests::MethodDescription &methodDescription,
                                const Tests::MethodTestCase &testCase);

        static Tests::MethodParam getValueParam(const Tests::MethodParam &param);
    };
}
#endif // UNITTESTBOT_TESTSPRINTER_H