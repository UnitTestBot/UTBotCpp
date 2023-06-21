#include "PrinterUtils.h"

#include "Paths.h"
#include "Tests.h"

#include "loguru.h"

namespace PrinterUtils {

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

    std::string getFieldAccess(const std::string &objectName, const types::Field &field) {
        if (field.name.empty()) {
            return objectName;
        }
        const std::string &fieldName = field.name;
        if (field.accessSpecifier == types::AccessSpecifier::AS_pubic) {
            if (fieldName.empty()) {
                return objectName;
            }
            return objectName + "." + fieldName;
        }
        return StringUtils::stringFormat("access_private::%s(%s)", fieldName, objectName);
    }

    std::string fillVarName(std::string const &access, std::string const &varName) {
        return StringUtils::stringFormat(access, varName);
    }

    std::string initializePointer(const std::string &type,
                                  const std::string &value,
                                  size_t additionalPointersCount) {
        if (value == C_NULL || std::stoull(value) == 0) {
            return C_NULL;
        } else {
            std::string additionalPointers = StringUtils::repeat("*", additionalPointersCount);
            return StringUtils::stringFormat("(%s%s) 0x%lx", type, additionalPointers,
                                             std::stoull(value));
        }
    }

    std::string initializePointerToVar(const std::string &type,
                                       const std::string &varName,
                                       size_t additionalPointersCount) {
        std::string additionalPointers = StringUtils::repeat("*", additionalPointersCount);
        return StringUtils::stringFormat("(%s%s) &%s", type, additionalPointers, varName);
    }

    std::string generateNewVar(int cnt) {
        return LAZYRENAME + std::to_string(cnt);
    }

    std::string getKleePrefix(bool forKlee) {
        return forKlee ? "klee_" : "";
    }

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

    std::string getFileParamKTestJSON(char fileName) {
        return StringUtils::stringFormat("%c-data", fileName);
    }

    std::string getFileReadBytesParamKTestJSON(char fileName) {
        return StringUtils::stringFormat("%c-data-read", fileName);
    }

    std::string getFileWriteBytesParamKTestJSON(char fileName) {
        return StringUtils::stringFormat("%c-data-write", fileName);
    }
}
