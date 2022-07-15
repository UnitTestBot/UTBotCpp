#ifndef UNITTESTBOT_PRINTERUTILS_H
#define UNITTESTBOT_PRINTERUTILS_H

#include "ProjectContext.h"

#include <grpcpp/grpcpp.h>
#include <protobuf/testgen.grpc.pb.h>

#include <optional>
#include <string>

#include "types/Types.h"
#include "utils/path/FileSystemPath.h"


namespace PrinterUtils {
    extern const std::string fromBytes;
    extern const std::string redirectStdin;

    extern const std::string DEFAULT_ACCESS;
    extern const std::string KLEE_PREFER_CEX;
    extern const std::string KLEE_ASSUME;
    extern const std::string KLEE_PATH_FLAG;
    extern const std::string KLEE_PATH_FLAG_SYMBOLIC;
    extern const std::string EQ_OPERATOR;
    extern const std::string ASSIGN_OPERATOR;
    extern const std::string TAB;

    extern const std::string EXPECTED;
    extern const std::string ACTUAL;
    extern const std::string ABS_ERROR;
    extern const std::string EXPECT_;
    extern const std::string EQ;

    std::string convertToBytesFunctionName(std::string const &typeName);

    std::string convertBytesToUnion(const std::string &typeName, const std::string &bytes);

    std::string wrapperName(const std::string &declName,
                            utbot::ProjectContext const &projectContext,
                            const fs::path& sourceFilePath);

    std::string getFieldAccess(std::string const& objectName, types::Field const &field);

    std::string getFieldAccess(std::string const &objectName, std::string const &fieldName);

    std::string fillVarName(std::string const &temp, std::string const &varName);

    std::string getFunctionPointerStubName(const std::optional<std::string> &scopeName,
                                           const std::string &methodName,
                                           const std::string &paramName,
                                           bool omitSuffix = false);

    std::string getFunctionPointerAsStructFieldStubName(const std::string &structName,
                                                        const std::string &fieldName,
                                                        bool omitSuffix = false);

    std::string getKleePrefix(bool forKlee);

    std::string wrapUserValue(const testsgen::ValidationType &type, const std::string &value);

    std::string getParamMangledName(const std::string& paramName, const std::string& methodName);
    std::string getReturnMangledName(const std::string& methodName);

    std::string getEqualString(const std::string& lhs, const std::string& rhs);
    std::string getDereferencePointer(const std::string& name, const size_t depth);
    std::string getExpectedVarName(const std::string& varName);

    std::string initializePointer(const std::string &type,
                                  const std::string &value,
                                  size_t additionalPointersCount);

    std::string initializePointerToVar(const std::string &type,
                                       const std::string &varName,
                                       size_t additionalPointersCount);

    std::string generateNewVar(int cnt);

    const std::string LAZYRENAME = "utbotInnerVar";
    const std::string UTBOT_ARGC = "utbot_argc";
    const std::string UTBOT_ARGV = "utbot_argv";
    const std::string UTBOT_ENVP = "utbot_envp";
    const std::string POSIX_INIT = "klee_init_env";
    const std::string WRAPPED_SUFFIX = "__wrapped";
    const std::string POSIX_CHECK_STDIN_READ = "check_stdin_read";
    const std::string MANGLED_PREFIX = "_Z";
    const std::string MANGLED_SUFFIX = "iPPcS0_";

    extern const std::string TEST_NAMESPACE;
    extern const std::string DEFINES_FOR_C_KEYWORDS;
    extern const std::string UNDEFS_FOR_C_KEYWORDS;
    extern const std::string KNOWN_IMPLICIT_RECORD_DECLS_CODE;
    extern const std::string C_NULL;
    extern const std::unordered_map <int, std::string> escapeSequences;

    extern const std::string KLEE_MODE;
    extern const std::string KLEE_SYMBOLIC_SUFFIX;
};


#endif // UNITTESTBOT_PRINTERUTILS_H
