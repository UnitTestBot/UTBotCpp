/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_PRINTER_H
#define UNITTESTBOT_PRINTER_H

#include "Language.h"
#include "StmtBordersFinder.h"
#include "Tests.h"
#include "building/BuildDatabase.h"
#include "stubs/Stubs.h"
#include "types/Types.h"

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

#define NL "\n"
#define SCNL ";" NL
#define BNL "{" NL
#define IND "i"
#define TAB "    "

namespace printer {
    using std::string;
    using std::vector;
    using std::unordered_map;
    using std::cout;
    using std::endl;

    using tests::Tests;

    class Printer {
    public:
        typedef const string &SRef;
        typedef const vector<string> &VSRef;
        typedef std::stringstream &Stream;

        std::stringstream ss;
        int tabsDepth = 0;
        utbot::Language srcLanguage = utbot::Language::UNKNOWN;

        Printer() = default;

        Printer(utbot::Language srcLanguage);

        virtual ~Printer() = default;

        virtual utbot::Language getLanguage() const;

        virtual bool needDecorate() const;

        void resetStream();

        string LB(bool startsWithSpace = true);

        string RB(bool needSC = false);

        inline string TAB_N() const {
            return StringUtils::repeat(TAB, tabsDepth);
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

        Stream strDeclareArrayVar(const types::Type& type,
                                  std::string_view name,
                                  types::PointerUsage usage,
                                  std::optional<std::string_view> value = std::nullopt,
                                  std::optional<uint64_t> alignment = std::nullopt,
                                  bool complete = true);

        Stream strAssignVar(std::string_view name, std::string_view value);

        std::stringstream& checkOverflowStubArray(const string &cntCall);

        Stream strTabIf(bool needTabs);

        Stream strFunctionDecl(
            SRef returnType,
            SRef functionName,
            vector<types::Type> const& paramTypes = {},
            VSRef paramValues = {},
            SRef end = SCNL,
            VSRef modifiers = {},
            const tests::Tests::MethodDescription::FPointerMap &fullDeclSubstitutions = {},
            bool isVariadic = false);

        Stream strFunctionDecl(const Tests::MethodDescription &method,
                               SRef end = SCNL,
                               VSRef modifiers = {});

        Stream strFunctionCall(std::string_view functionName,
                               const vector<string> &args,
                               const string &end = SCNL,
                               const std::optional<string> &classObj = std::nullopt,
                               bool needTabs = true,
                               size_t retPointers = 0,
                               std::optional<types::Type> castType = std::nullopt,
                               bool needComment = false);

        Stream strFunctionCall(const Tests::MethodDescription &method,
                               size_t returnPointers,
                               const string &end = SCNL,
                               bool needTabs = true);

        Stream writeCodeLine(std::string_view str);

        Stream strComment(SRef comment);

        Stream commentBlockSeparator();

        Stream closeBrackets(size_t sz);

        Stream gen2DPointer(const Tests::MethodParam &param, bool needDeclare);

        std::vector<string> printForLoopsAndReturnLoopIterators(SRef objectName,
                                                                const std::vector<size_t> &bounds);

        static string constrIndex(SRef arrayName, SRef ind);

        static string constrIndex(SRef arrayName, int ind);

        static string constrMultiIndex(SRef arrayName, const std::vector<size_t> &indexes);

        static string constrMultiIndex(SRef arrayName, const std::vector<string> &indexes);

        static string constrMultiIndex(const std::vector<string> &indexes);

        string constrFunctionCall(const string &functionName,
                                  const vector<string> &args,
                                  const string &end = "",
                                  const std::optional<string> &classObjName = std::nullopt,
                                  bool needTabs = true,
                                  size_t retPointers = 0,
                                  std::optional<types::Type> castType = std::nullopt);

        template<typename... Args>
        static string concat(Args&&... args) {
            std::stringstream cc_ss;
            (cc_ss << ... << args);
            return cc_ss.str();
        }

        [[nodiscard]] string recursiveIteratorName(SRef prefix) const;

        Stream strMemcpy(std::string_view dest,
                         std::string_view src,
                         bool needDereference = true);

        Stream strReturn(std::string_view value);

        Stream strTypedefFunctionPointer(const types::FunctionInfo& method, const string& name);

        Stream strDeclareArrayOfFunctionPointerVar(const string&arrayType, const string& arrayName,
                                                   const string& stubFunctionName);

        Stream strStubForMethod(const Tests::MethodDescription& method,
                                const types::TypesHandler&typesHandler,
                                const string& prefix,
                                const string& suffix,
                                bool makeStatic = false);

        static string getStubSymbolicVarName(const string& methodName);

        Stream strKleeMakeSymbolic(SRef varName, bool needAmpersand, SRef pseudoName);

        static inline std::string getTypedefFunctionPointer(const string& parentFunctionName,
                                                            const string& name,
                                                            bool isArray) {
            string parentFNameCopy = parentFunctionName;
            if (!parentFNameCopy.empty() && parentFNameCopy[0] == '&') {
                parentFNameCopy = parentFNameCopy.substr(1);
            }
            return StringUtils::stringFormat("%s_%s_arg%s", parentFNameCopy, name, isArray ? "_arr" : "");
        }

        static std::optional<string> getClassInstanceName(const std::optional<string>&className);

        string constrFunctionCall(const Tests::MethodDescription &method,
                                  size_t returnPointers,
                                  const string &end = "",
                                  bool needTabs = true);

        std::stringstream &strFunctionDeclWithParamString(const Tests::MethodDescription &method,
                                                          const string &end ,
                                                          const vector<string> &modifiers = {});

        void writeStubsForFunctionParams(const types::TypesHandler* typesHandler,
                                         const Tests::MethodDescription& testMethod,
                                         bool forKlee);

        void writeExternForSymbolicStubs(const Tests::MethodDescription& testMethod);

        void writeStubsForStructureFields(const Tests &tests);

        void writeStubForParam(const types::TypesHandler* typesHandler,
                               const std::shared_ptr<types::FunctionInfo> &fInfo,
                               const string& name,
                               const string& stubName, bool needToTypedef, bool makeStatic);

        void writePrivateAccessMacros(types::TypesHandler const *typesHandler, const Tests &tests);

        void genStubForStructFunctionPointer(const string& structName,
                                             const string& fieldName,
                                             const string& stubName);

        void genStubForStructFunctionPointerArray(const string& structName,
                                                  const string& fieldName,
                                                  const string& stubName);

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
