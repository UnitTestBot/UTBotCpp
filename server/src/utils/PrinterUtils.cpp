/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "PrinterUtils.h"

#include "Tests.h"
#include "Paths.h"
#include "loguru.h"

namespace PrinterUtils {
    const std::string fromBytes = "template<typename T, size_t N>\n"
                                  "T from_bytes(const char (&bytes)[N]) {\n"
                                  "    T result;\n"
                                  "    std::memcpy(&result, bytes, sizeof(result));\n"
                                  "    return result;\n"
                                  "}";

    const std::string redirectStdin = "void utbot_redirect_stdin(const char* buf, int &res) {\n"
                                      "    int fds[2];\n"
                                      "    if (pipe(fds) == -1) {\n"
                                      "        res = -1;\n"
                                      "        return;\n"
                                      "    }\n"
                                      "    close(STDIN_FILENO);\n"
                                      "    dup2(fds[0], STDIN_FILENO);\n"
                                      "    write(fds[1], buf, " + std::to_string(types::Type::symStdinSize) + ");\n"
                                      "    close(fds[1]);\n"
                                      "}";


    const std::string DEFAULT_ACCESS = "%s";
    const std::string KLEE_PREFER_CEX = "klee_prefer_cex";
    const std::string KLEE_ASSUME = "klee_assume";
    const std::string KLEE_PATH_FLAG = "kleePathFlag";
    const std::string KLEE_PATH_FLAG_SYMBOLIC = "kleePathFlagSymbolic";
    const std::string EQ_OPERATOR = " == ";
    const std::string ASSIGN_OPERATOR = " = ";
    const std::string TAB = "    ";

    const std::string EXPECTED = "expected";
    const std::string ACTUAL = "actual";
    const std::string ABS_ERROR = "utbot_abs_error";
    const std::string EXPECT_ = "EXPECT_";
    const std::string EQ = "EQ";

    std::string convertToBytesFunctionName(const std::string &typeName) {
        return StringUtils::stringFormat("from_bytes<%s>", typeName);
    }
    std::string convertBytesToUnion(const std::string &typeName, const std::string &bytes) {
        return StringUtils::stringFormat("%s(%s)", convertToBytesFunctionName(typeName), bytes);
    }

    std::string wrapperName(const std::string &declName,
                            utbot::ProjectContext const &projectContext,
                            const fs::path& sourceFilePath) {
        fs::path relativePath = fs::relative(sourceFilePath, projectContext.projectPath);
        std::string mangledPath = Paths::mangle(relativePath);
        return StringUtils::stringFormat("%s_%s", declName, mangledPath);
    }

    std::string getFieldAccess (std::string const& objectName, types::Field const &field) {
        if (field.name.empty()) {
            return objectName;
        }
        if (field.accessSpecifier == types::Field::AS_pubic) {
            return getFieldAccess(objectName, field.name);
        }
        return StringUtils::stringFormat("access_private::%s(%s)", field.name, objectName);
    }

    std::string getFieldAccess(std::string const& objectName, std::string const& fieldName) {
        if (fieldName.empty()) {
            return objectName;
        }
        return objectName + "." + fieldName;
    }

    std::string fillVarName(std::string const &access, std::string const &varName) {
        return StringUtils::stringFormat(access, varName);
    }

    std::string initializePointer(const std::string &type, const std::string &value) {
        if (value == C_NULL || std::stoull(value) == 0) {
            return C_NULL;
        } else {
            return StringUtils::stringFormat("(%s) 0x%x", type, std::stoull(value));
        }
    }

    std::string generateNewVar(int cnt) {
        return LAZYRENAME + std::to_string(cnt);
    }

    std::string getFunctionPointerStubName(const std::optional<std::string> &scopeName,
                                           const std::string &methodName,
                                           const std::string &paramName,
                                           bool omitSuffix) {
        std::string stubName = "*" + scopeName.value_or("") + "_" + methodName;
        if (!omitSuffix) {
            stubName += "_" + paramName + "_stub";
        } else {
            stubName = stubName.substr(1);
        }
        return stubName;
    }

    std::string getFunctionPointerAsStructFieldStubName(const std::string &structName,
                                                        const std::string &fieldName,
                                                        bool omitSuffix) {
        std::string stubName = "*" + structName;
        if (!omitSuffix) {
            stubName += "_" + fieldName + "_stub";
        } else {
            stubName = stubName.substr(1);
        }
        return stubName;
    }

    std::string getKleePrefix(bool forKlee) {
        return forKlee ? "klee_" : "";
    }

    const std::string TEST_NAMESPACE = "UTBot";
    const std::string DEFINES_FOR_C_KEYWORDS =
        /* Currently Clang tool transforms RecordDecl for
         * @code
         * struct data {
         * char x;
         * _Alignas(64) char cacheline[64];
         * };
         * to
         * @code
         * struct data {
         * char x;
         * char cacheline[64] _Alignas(64);
         * };
         * which is not valid code even for C, I suppose
         * */
        "#define _Alignas(x)\n"
        // can't be a part of function declaration, only typedef
        "#define _Atomic(x) x\n"
        "#define _Bool bool\n"
        // ignore for function declaration
        "#define _Noreturn\n"
        // can't be a part of function declaration, only typedef
        "#define _Thread_local thread_local\n"
        "";
    const std::string C_NULL = "NULL";
    // TODO This the list of known implicit records by now (i.e implicitly generated by the
    // implementation and not explicitly written in the source code). This particular is created in
    // ASTContext::buildImplicitRecord call in clang/lib/AST/ASTContext.cpp file. However, the
    // correct way would be to collect them while traversing types and write at the beginning of
    // header file.
    static const std::vector<std::string> KNOWN_IMPLICIT_RECORD_DECLS = { "struct __va_list_tag;" };
    const std::string KNOWN_IMPLICIT_RECORD_DECLS_CODE =
        StringUtils::joinWith(KNOWN_IMPLICIT_RECORD_DECLS, "\n");

    const std::unordered_map <int, std::string> escapeSequences = {
            {10, "\\n"},
            {9, "\\t"},
            {11, "\\v"},
            {8, "\\b"},
            {13, "\\r"},
            {12, "\\f"},
            {7, "\\a"},
            {92, "\\\\"},
            {63, "\\?"},
            {39, "\\\'"},
            {34, "\\\""},
            {0, "\\0"}
    };

    const std::string KLEE_MODE = "KLEE_MODE";
    const std::string KLEE_SYMBOLIC_SUFFIX = "_symbolic";

    std::string wrapUserValue(const testsgen::ValidationType &type, const std::string &value) {
        switch(type) {
            case testsgen::INT8_T:
            case testsgen::INT16_T:
            case testsgen::INT32_T:
            case testsgen::INT64_T:
            case testsgen::UINT8_T:
            case testsgen::UINT16_T:
            case testsgen::UINT32_T:
            case testsgen::UINT64_T:
            case testsgen::FLOAT:
            case testsgen::BOOL:
                return value;
            case testsgen::CHAR:
                return "\'" + value + "\'";
            case testsgen::STRING:
                return "\"" + value + "\"";
            default:
                ABORT_F("Unsupported ValidationType: %s", ValidationType_Name(type).c_str());
        }
    }

    std::string getParamMangledName(const std::string& paramName, const std::string& methodName) {
        return methodName + "_" + paramName + "_arg";
    }

    std::string getReturnMangledName(const std::string& methodName) {
        return methodName + "_return";
    }

    std::string getEqualString(const std::string& lhs, const std::string& rhs) {
        return StringUtils::stringFormat("%s == %s", lhs, rhs);
    }

    std::string getDereferencePointer(const std::string& name, const size_t depth) {
        return StringUtils::stringFormat("(%s%s)", StringUtils::repeat("*", depth), name);
    }

    std::string getExpectedVarName(const std::string& varName) {
        return "expected_" + varName;
    }
}
