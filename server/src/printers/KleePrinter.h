#ifndef UNITTESTBOT_KLEEPRINTER_H
#define UNITTESTBOT_KLEEPRINTER_H

#include "PathSubstitution.h"
#include "Printer.h"
#include "ProjectContext.h"
#include "Tests.h"
#include "LineInfo.h"
#include "building/BuildDatabase.h"
#include "types/Types.h"
#include "testgens/BaseTestGen.h"
#include "utils/path/FileSystemPath.h"

#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using tests::Tests;

namespace printer {
    class KleePrinter : public Printer {
    public:
        KleePrinter(const types::TypesHandler *typesHandler,
                    std::shared_ptr<BuildDatabase> buildDatabase,
                    utbot::Language srcLanguage, const BaseTestGen *testGen);

        utbot::Language getLanguage() const override;

        fs::path writeTmpKleeFile(
            const Tests &tests,
            const std::string &buildDir,
            const PathSubstitution &pathSubstitution,
            const std::optional<LineInfo::PredicateInfo> &predicateInfo = std::nullopt,
            const std::string &testedMethod = "",
            const std::optional<std::string> &testedClass = "",
            bool onlyForOneFunction = false,
            bool onlyForOneClass = false,
            const std::function<bool(tests::Tests::MethodDescription const &)> &methodFilter = [](tests::Tests::MethodDescription const &) { return true; });

        std::string addTestLineFlag(const std::shared_ptr<LineInfo> &lineInfo,
                                    bool needAssertion,
                                    const utbot::ProjectContext &projectContext);

    [[nodiscard]] std::vector<std::string> getIncludePaths(const Tests &tests, const PathSubstitution &substitution) const;
    private:
        types::TypesHandler const *typesHandler;
        BaseTestGen const *testGen;
        std::shared_ptr<BuildDatabase> buildDatabase;

        using PredInfo = LineInfo::PredicateInfo;
        struct ConstraintsState {
            std::string paramName;
            std::string curElement;
            types::Type curType;
        };

        void declTestEntryPoint(const Tests &tests, const Tests::MethodDescription &testMethod, bool isWrapped);

        void genGlobalParamsDeclarations(const Tests::MethodDescription &testMethod);

        void genPostParamsVariables(const Tests::MethodDescription &testMethod);

        void genParamsDeclarations(const Tests::MethodDescription &testMethod,
                                   std::function<bool(const tests::Tests::MethodParam &)> filter);

        bool genParamDeclaration(const Tests::MethodDescription &testMethod,
                                 const Tests::MethodParam &param);

        bool genPointerParamDeclaration(const Tests::MethodParam &param);

        void genReturnDeclaration(const Tests::MethodDescription &testMethod, const std::optional<PredInfo> &predicateInfo);

        void genParamsKleeAssumes(const Tests::MethodDescription &testMethod,
                                  const std::optional<PredInfo> &predicateInfo,
                                  const std::string &testedMethod,
                                  bool onlyForOneEntity);

        void genGlobalsKleeAssumes(const Tests::MethodDescription &testMethod);

        void
        genPostParamsKleeAssumes(const Tests::MethodDescription &testMethod,
                                 std::function<bool(const tests::Tests::MethodParam &)> filter);

        /*
         * Functions for constraints generation.
         */
        void genConstraints(const Tests::MethodParam &param, const std::string& methodName = "", const std::vector<std::string>& names = {});

        void genTwoDimPointers(const Tests::MethodParam &param, bool needDeclare);

        void genVoidFunctionAssumes(const Tests::MethodDescription &testMethod,
                                    const std::optional<PredInfo> &predicateInfo,
                                    const std::string &testedMethod,
                                    bool onlyForOneEntity);

        void genNonVoidFunctionAssumes(const Tests::MethodDescription &testMethod,
                                       const std::optional<PredInfo> &predicateInfo,
                                       const std::string &testedMethod,
                                       bool onlyForOneEntity);

        void genKleePathSymbolicIfNeeded(const std::optional<PredInfo> &predicateInfo,
                                         const std::string &testedMethod,
                                         bool onlyForOneEntity);

        void genKleePathSymbolicAssumeIfNeeded(const std::optional<PredInfo> &predicateInfo,
                                               const std::string &testedMethod,
                                               bool onlyForOneEntity);

        [[maybe_unused]] void addHeaderIncludeIfNecessary(std::unordered_set<std::string> &headers, const types::Type &type);

        Stream strKleeMakeSymbolic(SRef varName, bool needAmpersand);

        Stream strKleeMakeSymbolic(const types::Type &type, SRef varName, SRef pseudoName, bool needAmpersand);

        Stream strKleeMakeSymbolic(const types::Type &type, SRef varName, bool needAmpersand);

        void genPostGlobalSymbolicVariables(const Tests::MethodDescription &testMethod);

        void genPostParamsSymbolicVariables(
            const Tests::MethodDescription &testMethod,
            std::function<bool(const tests::Tests::MethodParam &)> filter);

        void makeBracketsForStrPredicate(const std::optional<PredInfo> &info);

        static Tests::MethodParam getKleeMethodParam(tests::Tests::MethodParam const &param);

        static Tests::MethodParam getKleePostParam(const Tests::MethodParam &param);

        static Tests::MethodParam getKleeGlobalParam(tests::Tests::MethodParam const &param);

        static Tests::MethodParam getKleeGlobalPostParam(const Tests::MethodParam &globalParam);

        void genPostSymbolicVariable(const Tests::MethodDescription &testMethod, const Tests::MethodParam &param);

        void genPostAssumes(const Tests::MethodParam &param, bool visitGlobal = false);

        void writePosixWrapper(const Tests &tests,
                               const tests::Tests::MethodDescription &testMethod);

        void writeTestedFunction(const Tests &tests,
                                 const tests::Tests::MethodDescription &testMethod,
                                 const std::optional<LineInfo::PredicateInfo> &predicateInfo,
                                 const std::string &testedMethod,
                                 bool onlyForOneEntity,
                                 bool isWrapped);

        void genOpenFiles(const tests::Tests::MethodDescription &testMethod);
    };
}

#endif //UNITTESTBOT_KLEEPRINTER_H
