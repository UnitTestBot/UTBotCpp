/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_PRINTERUTILS_H
#define UNITTESTBOT_PRINTERUTILS_H

#include "ProjectContext.h"

#include <grpcpp/grpcpp.h>
#include <protobuf/testgen.grpc.pb.h>

#include "utils/path/FileSystemPath.h"
#include <optional>
#include <string>


namespace PrinterUtils {
    extern const std::string fromBytes;
    extern const std::string redirectStdin;

    std::string convertToBytesFunctionName(std::string const &typeName);

    std::string convertBytesToUnion(const std::string &typeName, const std::string &bytes);

    std::string wrapperName(const std::string &declName,
                            utbot::ProjectContext const &projectContext,
                            const fs::path& sourceFilePath);

    std::string getFieldAccess(std::string const &objectName, std::string const &fieldName);

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

    std::string initializePointer(const std::string &type, const std::string &value);
    std::string generateNewVar(int cnt);

    const std::string LAZYRENAME = "utbotInnerVar";

    extern const std::string TEST_NAMESPACE;
    extern const std::string DEFINES_FOR_C_KEYWORDS;
    extern const std::string UNDEFS_FOR_C_KEYWORDS;
    extern const std::string C_NULL;
    extern const std::unordered_map <int, std::string> escapeSequences;

    extern const std::string KLEE_MODE;
    extern const std::string KLEE_SYMBOLIC_SUFFIX;
};


#endif // UNITTESTBOT_PRINTERUTILS_H
