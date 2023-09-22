#include "FeaturesFilter.h"

#include "TimeExecStatistics.h"
#include "clang-utils/ClangUtils.h"
#include "exceptions/NoTestGeneratedException.h"

#include "loguru.h"

static void updateIfNotCompleteType(types::TypeSupport &typeSupport,
                                    bool condition,
                                    std::string const &message) {
    if (typeSupport.isSupported && condition) {
        typeSupport = { false, message };
    }
}

bool hasPrivateArray(const types::Type &type, const types::TypesHandler &typesHandler) {
    switch (typesHandler.getTypeKind(type)) {
        case types::TypeKind::STRUCT_LIKE:
            for (const auto &field: typesHandler.getStructInfo(type).fields) {
                if (field.type.isArray() && field.accessSpecifier != types::AccessSpecifier::AS_pubic) {
                    return true;
                }
                return hasPrivateArray(field.type, typesHandler);
            }
            break;
        case types::TypeKind::OBJECT_POINTER:
        case types::TypeKind::ARRAY:
        case types::TypeKind::PRIMITIVE:
        case types::TypeKind::ENUM:
        case types::TypeKind::UNKNOWN:
        default:
            return false;
    }
    return false;
}

void FeaturesFilter::filter(utbot::SettingsContext const &settingsContext,
                            const types::TypesHandler &typesHandler,
                            tests::TestsMap &testsMap,
                            bool throwIfZeroFunctions) {
    bool hasSupportedMethods = false;
    std::unordered_map<std::string, int> unsupportedStatistics;
    for (auto testsMapIterator = testsMap.begin(); testsMapIterator != testsMap.end();
         testsMapIterator++) {
        fs::path const &sourceFile = testsMapIterator.key();
        tests::Tests &tests = testsMapIterator.value();

        filterGlobalParameters(typesHandler, tests);


        size_t erased = CollectionUtils::erase_if(tests.methods,
                     [&](const tests::Tests::MethodDescription &method) {
                         auto returnTypeSupport = typesHandler.isSupportedType(method.returnType, types::TypeUsage::RETURN);
                         updateIfNotCompleteType(returnTypeSupport, method.hasIncompleteReturnType,
                                                 "Method has incomplete return type");
                         if (!returnTypeSupport.isSupported) {
                             unsupportedStatistics[returnTypeSupport.info]++;
                             std::stringstream message;
                             message << "Function '" << method.name << "' was skipped, as return type '"
                                     << method.returnType.typeName()
                                     << "' is not fully supported: " << returnTypeSupport.info;
                             LOG_S(WARNING) << message.str();
                             tests.commentBlocks.push_back(message.str());
                             return true;
                         }

                         for (const auto &param: method.params) {
                             auto paramTypeSupport = typesHandler.isSupportedType(param.type, types::TypeUsage::PARAMETER);
                             updateIfNotCompleteType(paramTypeSupport, param.hasIncompleteType,
                                                     "Parameter has incomplete type");
                             if (!paramTypeSupport.isSupported) {
                                 unsupportedStatistics[paramTypeSupport.info]++;
                                 std::stringstream message;
                                 message << "Function '" << method.name << "' was skipped, as parameter '" << param.name
                                         << "' is not fully supported: " << paramTypeSupport.info;
                                 LOG_S(WARNING) << message.str();
                                 tests.commentBlocks.push_back(message.str());
                                 return true;
                             }
                         }

                         if (method.modifiers.isStatic && !settingsContext.generateForStaticFunctions) {
                             std::stringstream message;
                             message << "Function '" << method.name
                                     << "' was skipped, as option \"Generate For Static Functions\""
                                     << "is disabled";
                             LOG_S(DEBUG) << message.str();
                             tests.commentBlocks.push_back(message.str());
                             return true;
                         }

                         if (method.modifiers.isInline &&
                             (!method.modifiers.isStatic && !method.modifiers.isExtern)) {
                             std::stringstream message;
                             message
                                 << "Function '" << method.name
                                 << "' was skipped, as inline function without static or extern "
                                    "modifier is not supported by now";
                             LOG_S(DEBUG) << message.str();
                             tests.commentBlocks.push_back(message.str());
                             return true;
                         }

                         if (method.isClassMethod() &&
                             !typesHandler.getStructInfo(method.classObj->type).hasDefaultPublicConstructor) {
                             std::stringstream message;
                             message
                                     << "Method '" << method.name
                                     << "' was skipped, as class '" << method.getClassTypeName().value()
                                     << "' can't be construct in current version";
                             LOG_S(DEBUG) << message.str();
                             tests.commentBlocks.push_back(message.str());
                             return true;
                         }

                         if (method.isClassMethod() &&
                             hasPrivateArray(method.classObj->type, typesHandler)) {
                             std::stringstream message;
                             message
                                     << "Method '" << method.name
                                     << "' was skipped, as class '" << method.classObj->type.typeName()
                                     << "' has private field";
                             LOG_S(DEBUG) << message.str();
                             tests.commentBlocks.push_back(message.str());
                             return true;
                         }

                         for (const auto &param: method.params) {
                             if (typesHandler.isStructLike(param.type) && hasPrivateArray(param.type, typesHandler)) {
                                 std::stringstream message;
                                 message
                                         << "Method '" << method.name
                                         << "' was skipped, as parameter '" << param.type.typeName()
                                         << "' has private field";
                                 LOG_S(DEBUG) << message.str();
                                 tests.commentBlocks.push_back(message.str());
                                 return true;
                             }
                         }

                         if (typesHandler.isStructLike(method.returnType) && hasPrivateArray(method.returnType, typesHandler)) {
                             std::stringstream message;
                             message
                                     << "Method '" << method.name
                                     << "' was skipped, as return type '" << method.returnType.typeName()
                                     << "' has private field";
                             LOG_S(DEBUG) << message.str();
                             tests.commentBlocks.push_back(message.str());
                             return true;
                         }

                         if (method.accessSpecifier != types::AS_pubic) {
                             std::stringstream message;
                             message
                                     << "Method '" << method.name
                                     << "' from class '" << method.getClassTypeName().value_or("")
                                     << "' was skipped, as private";
                             LOG_S(DEBUG) << message.str();
                             tests.commentBlocks.push_back(message.str());
                             return true;
                         }

                         unsupportedStatistics["passed features filter"]++;

                         return false;
                     });
        LOG_S(DEBUG) << erased << " erased methods for file " << sourceFile;
        if (!tests.methods.empty()) {
            hasSupportedMethods = true;
        }
    }
    std::stringstream statsMessage;
    statsMessage << "Unsupported statistics:\n";
    for (const auto &[reason, count] : unsupportedStatistics) {
        statsMessage << StringUtils::stringFormat("%d functions: %s\n", count, reason);
    }
    LOG_S(DEBUG) << statsMessage.str();
    if (!hasSupportedMethods && throwIfZeroFunctions) {
        std::string message = "Couldn't find any supported methods. "
                              "Please check if source directories are specified correctly.";
        LOG_S(ERROR) << message;
        throw NoTestGeneratedException(message);
    }
}

static void logReason(const types::TypeName& typeName, std::string_view methodName, std::string_view info) {
    LOG_S(DEBUG) << "Global parameter " << typeName
                 << " for function \'" << methodName
                 << "\' was skipped: " << info;
}

void FeaturesFilter::filterGlobalParameters(const types::TypesHandler &typesHandler,
                                            tests::Tests &tests) {
    for (auto it = tests.methods.begin(); it != tests.methods.end(); it++) {
        auto &methodName = it.key();
        auto &methodDescription = it.value();
        size_t erased = CollectionUtils::erase_if(
            methodDescription.globalParams, [&](tests::Tests::MethodParam const &param) {
                auto globalParamSupport =
                    typesHandler.isSupportedType(param.type, types::TypeUsage::PARAMETER);
                if (!globalParamSupport.isSupported) {
                    logReason(param.name, methodName, globalParamSupport.info);
                    return true;
                }
                if (param.type.baseTypeObj().isPointerToFunction()) {
                    logReason(param.name, methodName, "Type or its base is 'pointer to function'");
                    return true;
                }
                if (param.type.baseTypeObj().isUnnamed()) {
                    logReason(param.name, methodName, "Type or its base is unnamed");
                    return true;
                }
                if (typesHandler.isVoid(param.type.baseTypeObj())) {
                    logReason(param.name, methodName, "Type or its base is void");
                    return true;
                }
                if (typesHandler.isIncompleteArrayType(param.type)) {
                    logReason(param.name, methodName, "Type is incomplete array");
                    return true;
                }
                if (typesHandler.isArrayType(param.type) && param.type.baseTypeObj(1).isObjectPointer()) {
                    logReason(param.name, methodName, "Type is array of pointers");
                    return true;
                }
                return false;
            });
        LOG_S(DEBUG) << erased << " erased global variables for method " << methodName;
    }
}
