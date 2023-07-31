#ifndef UNITTESTBOT_PRINTERUTILS_H
#define UNITTESTBOT_PRINTERUTILS_H

#include "ProjectContext.h"
#include "types/Types.h"
#include "utils/path/FileSystemPath.h"

#include <grpcpp/grpcpp.h>
#include <protobuf/testgen.grpc.pb.h>

#include <optional>
#include <string>

namespace PrinterUtils {
    const std::string fromBytes = "template<typename T, size_t N>\n"
                                  "T from_bytes(const char (&bytes)[N]) {\n"
                                  "    T result;\n"
                                  "    std::memcpy(&result, bytes, sizeof(result));\n"
                                  "    return result;\n"
                                  "}\n";

    const std::string redirectStdin = "void utbot_redirect_stdin(const char* buf, int &res) {\n"
                                      "    int fds[2];\n"
                                      "    if (pipe(fds) == -1) {\n"
                                      "        res = -1;\n"
                                      "        return;\n"
                                      "    }\n"
                                      "    close(STDIN_FILENO);\n"
                                      "    dup2(fds[0], STDIN_FILENO);\n"
                                      "    write(fds[1], buf, " +
                                      std::to_string(types::Type::symInputSize) +
                                      ");\n"
                                      "    close(fds[1]);\n"
                                      "}\n";

    const std::string writeToFile = "void write_to_file(const char *fileName, const char *buf) {\n"
                                    "    FILE *out = fopen(fileName, \"w\");\n"
                                    "    if (out == NULL) {\n"
                                    "        return;\n"
                                    "    }\n"
                                    "    fwrite(buf, 1, " +
                                    std::to_string(types::Type::symInputSize) +
                                    ", out);\n"
                                    "    fclose(out);\n"
                                    "}\n";

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
    const std::string EXPECT_ = "EXPECT_";
    const std::string EXPECT_FLOAT_EQ = "EXPECT_FLOAT_EQ";
    const std::string EXPECT_DOUBLE_EQ = "EXPECT_DOUBLE_EQ";
    const std::string EQ = "EQ";

    std::string convertToBytesFunctionName(std::string const &typeName);

    std::string convertBytesToUnion(const std::string &typeName, const std::string &bytes);

    std::string wrapperName(const std::string &declName,
                            utbot::ProjectContext const &projectContext,
                            const fs::path &sourceFilePath);

    std::string getFieldAccess(const std::string &objectName, const types::Field &field);

    std::string fillVarName(std::string const &temp, std::string const &varName);

    std::string getKleePrefix(bool forKlee);

    std::string wrapUserValue(const testsgen::ValidationType &type, const std::string &value);

    std::string getParamMangledName(const std::string &paramName, const std::string &methodName);
    std::string getReturnMangledName(const std::string &methodName);
    std::string getReturnMangledTypeName(const std::string& methodName);
    std::string getEnumReturnMangledTypeName(const std::string& methodName);

    void decorateBaseTypeNameWithNamespace(types::TypeName &typeName,
                                           const types::TypeName &baseTypeName);
    void decorateTypeNameWithNamespace(uint64_t id, types::TypeName &typeName,
                                       const types::TypesHandler* typesHandler);

    std::string getEqualString(const std::string &lhs, const std::string &rhs);
    std::string getDereferencePointer(const std::string &name, const size_t depth);
    std::string getExpectedVarName(const std::string &varName);

    std::string initializePointer(const std::string &type,
                                  const std::string &value,
                                  size_t additionalPointersCount);

    std::string initializePointerToVar(const std::string &type,
                                       const std::string &varName,
                                       size_t additionalPointersCount);

    std::string generateNewVar(int cnt);

    std::string getFileParamKTestJSON(char fileName);
    std::string getFileReadBytesParamKTestJSON(char fileName);
    std::string getFileWriteBytesParamKTestJSON(char fileName);

    const std::string LAZYRENAME = "utbotInnerVar";
    const std::string UTBOT_ARGC = "utbot_argc";
    const std::string UTBOT_ARGV = "utbot_argv";
    const std::string UTBOT_ENVP = "utbot_envp";
    const std::string POSIX_INIT = "klee_init_env";
    const std::string WRAPPED_SUFFIX = "__wrapped";
    const std::string POSIX_CHECK_STDIN_READ = "check_stdin_read";
    const std::string MANGLED_PREFIX = "_Z";
    const std::string MANGLED_SUFFIX = "iPPcS0_";

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

    // TODO This the list of known implicit records by now (i.e implicitly generated by the
    // implementation and not explicitly written in the source code). This particular is created in
    // ASTContext::buildImplicitRecord call in clang/lib/AST/ASTContext.cpp file. However, the
    // correct way would be to collect them while traversing types and write at the beginning of
    // header file.
    static const std::vector<std::string> KNOWN_IMPLICIT_RECORD_DECLS = { "struct __va_list_tag;" };
    const std::string KNOWN_IMPLICIT_RECORD_DECLS_CODE =
        StringUtils::joinWith(KNOWN_IMPLICIT_RECORD_DECLS, "\n");

    const std::string C_NULL = "NULL";
    const std::unordered_map<int, std::string> escapeSequences = {
        { 10, "\\n" }, { 9, "\\t" },   { 11, "\\v" }, { 8, "\\b" },   { 13, "\\r" },  { 12, "\\f" },
        { 7, "\\a" },  { 92, "\\\\" }, { 63, "\\?" }, { 39, "\\\'" }, { 34, "\\\"" }, { 0, "\\0" }
    };

    const std::string KLEE_MODE = "KLEE_MODE";
    const std::string KLEE_SYMBOLIC_SUFFIX = "_symbolic";
};

#endif // UNITTESTBOT_PRINTERUTILS_H
