#ifndef UNITTESTBOT_PRINTER_H
#define UNITTESTBOT_PRINTER_H

#include "Language.h"
#include "Tests.h"
#include "building/BuildDatabase.h"
#include "stubs/Stubs.h"
#include "utils/path/FileSystemPath.h"
#include "types/Types.h"

#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define NL "\n"
#define SCNL ";" NL
#define BNL "{" NL
#define IND "i"
#define TAB "    "

namespace printer {
    using tests::Tests;

    class Printer {
    public:
        typedef const std::string &SRef;
        typedef const std::vector<std::string> &VSRef;
        typedef std::stringstream &Stream;

        std::stringstream ss;
        int tabsDepth = 0;
        int commentDepth = 0;
        utbot::Language srcLanguage = utbot::Language::UNKNOWN;

        Printer() = default;

        Printer(utbot::Language srcLanguage);

        virtual ~Printer() = default;

        virtual utbot::Language getLanguage() const;

        virtual bool needDecorate() const;

        void resetStream();

        std::string LB(bool startsWithSpace = true);

        std::string RB(bool needSC = false);

        inline std::string LINE_INDENT() const {
            std::string tabs = StringUtils::repeat(TAB, tabsDepth);
            return commentDepth <= 0
               ? tabs
               : tabs + "// ";
        }

        // all functions which return std::stringstream start with `str` prefix.
        // all functions which return std::string start with `constr` prefix.

        Stream strDefine(std::string_view from, std::string_view to);

        Stream strInclude(SRef header, bool isAngled = false);
        Stream strInclude(const Include& include);


        Stream strIncludeSystem(SRef header);

        Stream strForBound(SRef it, size_t n);

        Stream strIfBound(SRef condition);

        Stream strDeclareVar(std::string_view type,
                             std::string_view name,
                             std::optional<std::string_view> initValue = std::nullopt,
                             std::optional<uint64_t> alignment = std::nullopt,
                             bool complete = true,
                             size_t additionalPointersCount = 0);

        Stream strDeclareAbsError(SRef name);

        enum ExternType {
            NONE,
            SAME_LANGUAGE,
            C
        };

        Stream strDeclareArrayVar(const types::Type& type,
                                  std::string_view name,
                                  types::PointerUsage usage,
                                  std::optional<std::string_view> value = std::nullopt,
                                  std::optional<uint64_t> alignment = std::nullopt,
                                  bool complete = true,
                                  ExternType externType = ExternType::NONE);

        Stream strDeclareSetOfVars(const std::set<Tests::TypeAndVarName> &vars);

        Stream strAssignVar(std::string_view name, std::string_view value);

        std::stringstream& checkOverflowStubArray(const std::string &cntCall);

        Stream strTabIf(bool needTabs);

        Stream strFunctionDecl(
            SRef returnType,
            SRef functionName,
            std::vector<types::Type> const& paramTypes = {},
            VSRef paramValues = {},
            SRef end = SCNL,
            VSRef modifiers = {},
            const tests::Tests::MethodDescription::FPointerMap &fullDeclSubstitutions = {},
            bool isVariadic = false);

        Stream strFunctionDecl(const Tests::MethodDescription &method,
                               SRef end = SCNL,
                               VSRef modifiers = {});

        Stream strFunctionCall(std::string_view functionName,
                               const std::vector<std::string> &args,
                               const std::string &end = SCNL,
                               const std::optional<std::string> &classObj = std::nullopt,
                               bool needTabs = true,
                               size_t retPointers = 0,
                               std::optional<types::Type> castType = std::nullopt,
                               bool needComment = false);

        Stream strFunctionCall(const Tests::MethodDescription &method,
                               size_t returnPointers,
                               const std::string &end = SCNL,
                               bool needTabs = true);

        Stream writeCodeLine(std::string_view str);

        Stream strComment(SRef comment);

        Stream commentBlockSeparator();

        Stream closeBrackets(size_t sz);

        Stream gen2DPointer(const Tests::MethodParam &param, bool needDeclare);

        std::vector<std::string> printForLoopsAndReturnLoopIterators(const std::vector<size_t> &bounds);

        static std::string constrIndex(SRef arrayName, SRef ind);

        static std::string constrIndex(SRef arrayName, int ind);

        static std::string constrMultiIndex(SRef arrayName, const std::vector<size_t> &indexes);

        static std::string constrMultiIndex(SRef arrayName, const std::vector<std::string> &indexes);

        static std::string constrMultiIndex(const std::vector<std::string> &indexes);

        std::string constrFunctionCall(const std::string &functionName,
                                       const std::vector<std::string> &args,
                                       const std::string &end = "",
                                       const std::optional<std::string> &classObjName = std::nullopt,
                                       bool needTabs = true,
                                       size_t retPointers = 0,
                                       std::optional<types::Type> castType = std::nullopt);

        template<typename... Args>
        static std::string concat(Args&&... args) {
            std::stringstream cc_ss;
            (cc_ss << ... << args);
            return cc_ss.str();
        }

        [[nodiscard]] std::string recursiveIteratorName(SRef prefix) const;

        Stream strMemcpy(std::string_view dest,
                         std::string_view src,
                         bool needDereference = true);

        Stream strReturn(std::string_view value);

        Stream strTypedefFunctionPointer(const types::FunctionInfo &method, const std::string &name);

        Stream strDeclareArrayOfFunctionPointerVar(const std::string &arrayType, const std::string &arrayName,
                                                   const std::string &stubFunctionName);

        Stream strStubForMethod(const Tests::MethodDescription &method,
                                const types::TypesHandler &typesHandler,
                                const std::string &prefix,
                                const std::string &suffix,
                                const std::string &parentMethodName,
                                bool makeStatic);

        Stream strKleeMakeSymbolic(SRef varName, bool needAmpersand, SRef pseudoName);

        static inline std::string getTypedefFunctionPointer(const std::string &parentFunctionName,
                                                            const std::string &name,
                                                            bool isArray) {
            std::string parentFNameCopy = parentFunctionName;
            if (!parentFNameCopy.empty() && parentFNameCopy[0] == '&') {
                parentFNameCopy = parentFNameCopy.substr(1);
            }
            return StringUtils::stringFormat("%s_%s_arg%s", parentFNameCopy, name, isArray ? "_arr" : "");
        }

        std::string constrFunctionCall(const Tests::MethodDescription &method,
                                       size_t returnPointers,
                                       const std::string &end = "",
                                       bool needTabs = true);

        void writeStubsForFunctionParams(const types::TypesHandler* typesHandler,
                                         const Tests::MethodDescription& testMethod,
                                         bool forKlee);

        void writeExternForSymbolicStubs(const Tests::MethodDescription& testMethod);

        void writeStubsForStructureFields(const Tests &tests);

        void writeStubsForParameters(const Tests &tests);

        void writeStubForParam(const types::TypesHandler* typesHandler,
                               const std::shared_ptr<types::FunctionInfo> &fInfo,
                               const std::string &methodName,
                               const std::string &stubName, bool needToTypedef, bool makeStatic);

        void writeAccessPrivateMacros(types::TypesHandler const *typesHandler, const Tests &tests, bool onlyChangeable,
                                      const std::function<bool(tests::Tests::MethodDescription const &)> &methodFilter);

        void writeAccessPrivateMacros(types::TypesHandler const *typesHandler, const Tests &tests, bool onlyChangeable);

        void genStubForStructFunctionPointer(const std::string &structName,
                                             const types::Field &fieldName,
                                             const std::string &stubName);

        void genStubForStructFunctionPointerArray(const std::string &structName,
                                                  const types::Field &fieldName,
                                                  const std::string &stubName);

        static std::string getConstQualifier(const types::Type& type);

    private:
        Stream strMemcpyImpl(std::string_view dest, std::string_view src, bool needDereference);

        void printAlignmentIfExists(const std::optional<uint64_t> &alignment);

        void addAccessor(const types::TypesHandler *typesHandler, const types::Type &type,
                         std::unordered_set<uint64_t> &checkedOnPrivate);

    protected:
        virtual void writeCopyrightHeader();
    };
}
#endif //UNITTESTBOT_PRINTER_H
