#include "Tests.h"

#include "NameDecorator.h"
#include "exceptions/UnImplementedException.h"
#include "printers/TestsPrinter.h"
#include "utils/KleeUtils.h"
#include "utils/StringUtils.h"
#include "utils/StubsUtils.h"

#include "loguru.h"

#include <algorithm>
#include <iterator>

using namespace tests;
using namespace types;

static const std::string INT64_MIN_STRING =
    std::to_string(std::numeric_limits<int64_t>::min());

const std::string Tests::DEFAULT_SUITE_NAME = "regression";
const std::string Tests::ERROR_SUITE_NAME = "error";

const Tests::MethodParam &tests::Tests::getStdinMethodParam() {
    static const Tests::MethodParam stdinMethodParam =
            MethodParam(types::Type::CStringType(), types::Type::getStdinParamName(), std::nullopt);
    return stdinMethodParam;
}

Tests::MethodDescription::MethodDescription()
        : suiteTestCases{{ Tests::DEFAULT_SUITE_NAME, std::vector<int>() },
                         { Tests::ERROR_SUITE_NAME,   std::vector<int>() }},
          codeText{{ Tests::DEFAULT_SUITE_NAME, std::string() },
                   { Tests::ERROR_SUITE_NAME,   std::string() }},
          modifiers{} {
    stubsStorage = std::make_shared<StubsStorage>();
}

static const std::unordered_map<std::string, std::string> FPSpecialValuesMappings = {
    {"nan", "NAN"},
    {"-nan", "-NAN"},
    {"inf", "INFINITY"},
    {"-inf", "-INFINITY"}
};

static std::string makeDecimalConstant(std::string value, const std::string &typeName) {
    if (typeName == "long") {
        if (value == INT64_MIN_STRING) {
            return "(-9223372036854775807L - 1)";
        }
        return value + "L";
    }
    if (typeName == "long long") {
        if (value == INT64_MIN_STRING) {
            return "(-9223372036854775807LL - 1)";
        }
        return value + "LL";
    }
    if (typeName == "unsigned int") {
        return value + "U";
    }
    if (typeName == "unsigned long") {
        return value + "UL";
    }
    if (typeName == "unsigned long long") {
        return value + "ULL";
    }
    if (typeName == "long double") {
        if ( FPSpecialValuesMappings.find(value) == FPSpecialValuesMappings.end()) {
            // we need it to avoid overflow in exponent for const like 1.18973e+4932L
            // BUT! Skip the NAN/INFINITY values
            return value + "L";
        }
    }
    return value;
}

namespace tests {
/**
 * The function checks for presence of argument in values as it is
 * called by the time processFPSpecialValue is already applied
*/
bool isFPSpecialValue(const std::string& value) {
    return CollectionUtils::contains(CollectionUtils::getValues(FPSpecialValuesMappings), value);
}

/**
 *  We need to change representation of special values,
 *  because code float f = nan; float f = inf; does not compile
*/
std::string processFPSpecialValue(const std::string &value) {
    if (CollectionUtils::containsKey(FPSpecialValuesMappings, value)) {
        return FPSpecialValuesMappings.at(value);
    } else {
        return value;
    }
}

std::shared_ptr<PrimitiveValueView> KTestObjectParser::primitiveView(const std::vector<char> &byteArray,
                                                                     const types::Type &type,
                                                                     size_t offsetInBits,
                                                                     size_t lenInBits) {
    Type readType = types::TypesHandler::isVoid(type) ? Type::minimalScalarType() : type;
    std::string value = readBytesAsValueForType(byteArray, readType.baseType(), offsetInBits, lenInBits);
    value = makeDecimalConstant(value, type.baseType());
    value = processFPSpecialValue(value);
    if (types::TypesHandler::isBoolType(type)) {
        return std::make_shared<PrimitiveValueView>(primitiveBoolView(value));
    }
    return std::make_shared<PrimitiveValueView>(primitiveCharView(type.baseTypeObj(), value));
}


std::shared_ptr<EnumValueView> KTestObjectParser::enumView(const std::vector<char> &byteArray,
                                                           const types::EnumInfo &enumInfo,
                                                           size_t offsetInBits,
                                                           size_t lenInBits) {
    std::string value = readBytesAsValue<int>(byteArray, offsetInBits, lenInBits);
    if (CollectionUtils::containsKey(enumInfo.valuesToEntries, value)) {
        auto name = enumInfo.getEntryName(value, utbot::Language::CXX);
        value = NameDecorator::decorate(name);
    } else {
        LOG_S(WARNING) << "Enum value for '" << enumInfo.name << "' is out of range: " << value;
        std::string format = enumInfo.isSpecifierNeeded ? "(enum %s)(%d)" : "(%s) %d";
        value = StringUtils::stringFormat(format, enumInfo.name, value);
    }
    return std::make_shared<EnumValueView>(value);
}

std::shared_ptr<StringValueView> KTestObjectParser::stringLiteralView(const std::vector<char> &byteArray,
                                                                      size_t length) {
    std::string value = "\"";
    bool skip = (length == 0);
    if (length == 0) {
        length = byteArray.size();
    }
    for (size_t i = 0; i < length; i++) {
        char c = byteArray[i];
        if (c == '\0' && skip) {
            break; //prefer the shortest example
        } else {
            value += StringUtils::charCodeToLiteral(static_cast<int>(c));
        }
        if (!StringUtils::isPrintable(static_cast<int>(c)) && i + 1 < byteArray.size()) {
            value += "\"\"";
        }
    }
    value.push_back('\"');
    return std::make_shared<StringValueView>(value);
}

std::shared_ptr<ArrayValueView> KTestObjectParser::multiArrayView(const std::vector<char> &byteArray,
                                                                  const std::vector<Pointer> &lazyPointersArray,
                                                                  const types::Type &type,
                                                                  size_t arraySizeInBits,
                                                                  size_t offsetInBits,
                                                                  PointerUsage usage) {
    std::vector<std::shared_ptr<AbstractValueView>> views;
    const types::Type baseType = type.baseTypeObj();

    size_t elementLenInBits = (types::TypesHandler::isVoid(baseType))
                              ? typesHandler.typeSize(Type::minimalScalarType())
                              : typesHandler.typeSize(baseType);

    for (size_t curPos = offsetInBits; curPos < offsetInBits + arraySizeInBits; curPos += elementLenInBits) {
        switch (typesHandler.getTypeKind(baseType)) {
        case TypeKind::STRUCT_LIKE:
            views.push_back(structView(byteArray, lazyPointersArray, typesHandler.getStructInfo(baseType), curPos, usage));
            break;
        case TypeKind::ENUM:
            views.push_back(enumView(byteArray, typesHandler.getEnumInfo(type), curPos, elementLenInBits));
            break;
        case TypeKind::PRIMITIVE:
            views.push_back(primitiveView(byteArray, baseType, curPos, elementLenInBits));
            break;
        case TypeKind::OBJECT_POINTER:
        case TypeKind::ARRAY:
            LOG_S(ERROR) << "Invariant ERROR: base type is pointer/array: " << type.typeName();
            // No break here
        case TypeKind::UNKNOWN:
            throw UnImplementedException(
                std::string("Arrays don't support element type: " + type.typeName())
            );
        default:
            std::string message = "Missing case for this TypeKind in switch";
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
    }

    std::vector<size_t> sizes = type.arraysSizes(usage);
    for (size_t i = sizes.size() - 1; i > 0; i--) {
        size_t size = sizes[i];
        std::vector<std::shared_ptr<AbstractValueView>> newViews;
        for (size_t j = 0; j < views.size(); j += size) {
            std::vector<std::shared_ptr<AbstractValueView>> curViews =
                std::vector(views.begin() + j, views.begin() + j + size);
            newViews.push_back(std::make_shared<ArrayValueView>(curViews));
        }
        views = newViews;
    }

    return std::make_shared<ArrayValueView>(views);
}

std::shared_ptr<FunctionPointerView> KTestObjectParser::functionPointerView(const std::optional<std::string> &scopeName,
                                                                            const std::string &methodName, const std::string &paramName) {
    std::string value =
        StubsUtils::getFunctionPointerStubName(scopeName, methodName, paramName).substr(1);
    return std::make_shared<FunctionPointerView>(value);
}

std::shared_ptr<FunctionPointerView> KTestObjectParser::functionPointerView(const std::string &structName,
                                                                            const std::string &fieldName) {
    std::string value = StubsUtils::getFunctionPointerAsStructFieldStubName(structName, fieldName, false).substr(1);
    return std::make_shared<FunctionPointerView>(value);
}

std::shared_ptr<ArrayValueView> KTestObjectParser::arrayView(const std::vector<char> &byteArray,
                                                             const std::vector<Pointer> &lazyPointersArray,
                                                             const types::Type &type,
                                                             size_t arraySizeInBits,
                                                             size_t offsetInBits,
                                                             PointerUsage usage) {
    std::vector<std::shared_ptr<AbstractValueView>> subViews;
    size_t elementLenInBits = (types::TypesHandler::isVoid(type))
                              ? typesHandler.typeSize(Type::minimalScalarType())
                              : typesHandler.typeSize(type);

    for (size_t curPos = offsetInBits; curPos < offsetInBits + arraySizeInBits; curPos += elementLenInBits) {
        switch (typesHandler.getTypeKind(type)) {
            case TypeKind::STRUCT_LIKE:
                subViews.push_back(structView(byteArray, lazyPointersArray, typesHandler.getStructInfo(type), curPos, usage));
                break;
            case TypeKind::ENUM:
                subViews.push_back(enumView(byteArray, typesHandler.getEnumInfo(type), curPos, elementLenInBits));
                break;
            case TypeKind::PRIMITIVE:
                subViews.push_back(primitiveView(byteArray, type.baseTypeObj(), curPos, elementLenInBits));
                break;
            case TypeKind::OBJECT_POINTER:
            case TypeKind::ARRAY:
            case TypeKind::UNKNOWN:
                throw UnImplementedException(
                    std::string("Arrays don't support element type: " + type.typeName())
                );
            default:
                std::string message = "Missing case for this TypeKind in switch";
                LOG_S(ERROR) << message;
                throw NoSuchTypeException(message);
        }
    }
    return std::make_shared<ArrayValueView>(subViews);
}

std::shared_ptr<StructValueView> KTestObjectParser::structView(const std::vector<char> &byteArray,
                                                               const std::vector<Pointer> &lazyPointersArray,
                                                               const types::StructInfo &curStruct,
                                                               size_t offsetInBits,
                                                               types::PointerUsage usage) {
    std::vector<InitReference> tmpInitReferences;
    return structView(byteArray, lazyPointersArray, curStruct, offsetInBits, usage, {}, false, "", {}, tmpInitReferences);
}

std::shared_ptr<StructValueView> KTestObjectParser::structView(const std::vector<char> &byteArray,
                                                               const std::vector<Pointer> &lazyPointersArray,
                                                               const StructInfo &curStruct,
                                                               size_t offsetInBits,
                                                               PointerUsage usage,
                                                               const std::optional<const Tests::MethodDescription> &testingMethod,
                                                               const bool anonymousField,
                                                               const std::string &name,
                                                               const std::vector<UTBotKTestObject> &objects,
                                                               std::vector<InitReference> &initReferences) {
    std::vector<std::shared_ptr<AbstractValueView>> subViews;

    size_t fieldIndexToInitUnion = SIZE_MAX;
    size_t sizeOfFieldToInitUnion = 0;
    size_t prevFieldEndOffset = offsetInBits;
    size_t structEndOffset = offsetInBits + curStruct.size;
    size_t fieldIndex = 0;
    bool dirtyInitializedStruct = false;
    bool isInitializedStruct = curStruct.subType == types::SubType::Struct;
    for (const auto &field: curStruct.fields) {
        bool dirtyInitializedField = false;
        bool isInitializedField = true;
        size_t fieldLen = typesHandler.typeSize(field.type);
        size_t fieldStartOffset = offsetInBits + field.offset;
        size_t fieldEndOffset = fieldStartOffset + fieldLen;
        if (curStruct.subType == types::SubType::Union) {
            prevFieldEndOffset = offsetInBits;
        }

        auto dirtyCheck = [&](size_t i) {
            if (i >= byteArray.size()) {
                LOG_S(ERROR) << "Bad type size info: " << field.name << " index: " << fieldIndex;
            } else if (byteArray[i] == 0) {
                return false;
            }
            // the field cannot init the union in this state
            dirtyInitializedField = true;
            return true;
        };

        if (prevFieldEndOffset < fieldStartOffset) {
            // check an alignment gap
            for (size_t i = prevFieldEndOffset / 8; i < fieldStartOffset / 8; ++i) {
                if (dirtyCheck(i)) {
                    break;
                }
            }
        }
        if (!dirtyInitializedField && (curStruct.subType == types::SubType::Union ||
                                       fieldIndex + 1 == curStruct.fields.size())) {
            // check the rest of the union or the last field of the struct
            for (size_t i = fieldEndOffset / 8; i < structEndOffset / 8; ++i) {
                if (dirtyCheck(i)) {
                    break;
                }
            }
        }

        switch (typesHandler.getTypeKind(field.type)) {
            case TypeKind::STRUCT_LIKE:
                {
                    auto sv = structView(byteArray, lazyPointersArray, typesHandler.getStructInfo(field.type),
                                         fieldStartOffset, usage, testingMethod, field.anonymous,
                                         PrinterUtils::getFieldAccess(name, field), objects,
                                         initReferences);
                    dirtyInitializedField |= sv->isDirtyInit();
                    isInitializedField = sv->isInitialized();
                    subViews.push_back(sv);
                }
                break;
            case TypeKind::ENUM:
                subViews.push_back(enumView(byteArray, typesHandler.getEnumInfo(field.type),
                                            fieldStartOffset, fieldLen));
                break;
            case TypeKind::PRIMITIVE:
                subViews.push_back(primitiveView(byteArray, field.type.baseTypeObj(),
                                                 fieldStartOffset,
                                                 std::min(field.size, fieldLen)));
                break;
            case TypeKind::ARRAY: {
                const std::vector<std::shared_ptr<AbstractType>> pointerArrayKinds = field.type.pointerArrayKinds();
                if (pointerArrayKinds.size() > 1) {
                    size_t size = 1;
                    bool onlyArrays = true;
                    for (const auto &pointerArrayKind : pointerArrayKinds) {
                        if (pointerArrayKind->getKind() == AbstractType::ARRAY) {
                            size *= pointerArrayKind->getSize();
                        } else {
                            onlyArrays = false;
                            break;
                        }
                    }
                    if (onlyArrays) {
                        size *= typesHandler.typeSize(field.type.baseTypeObj());
                        subViews.push_back(multiArrayView(byteArray, lazyPointersArray, field.type, size, fieldStartOffset, usage));
                    } else {
                        std::vector<std::shared_ptr<AbstractValueView>> nullViews(
                            size, std::make_shared<JustValueView>(PrinterUtils::C_NULL));
                        subViews.push_back(std::make_shared<ArrayValueView>(nullViews));
                    }
                } else {
                    auto view = arrayView(byteArray, lazyPointersArray, field.type.baseTypeObj(), fieldLen,
                                          fieldStartOffset, usage);
                    subViews.push_back(view);
                }
            }
                break;
            case TypeKind::OBJECT_POINTER: {
                std::string res = readBytesAsValueForType(byteArray, PointerWidthType,
                                                          fieldStartOffset, PointerWidthSizeInBits);
                auto pointerIterator =
                    std::find_if(lazyPointersArray.begin(), lazyPointersArray.end(),
                                 [&fieldStartOffset](const Pointer &ptr) {
                                     return SizeUtils::bytesToBits(ptr.offset) == fieldStartOffset;
                                 });
                subViews.push_back(getLazyPointerView(
                    objects, initReferences, PrinterUtils::getFieldAccess(name, field), res,
                    field.type, pointerIterator != lazyPointersArray.end()));
            } break;
            case TypeKind::FUNCTION_POINTER:
                subViews.push_back(functionPointerView(curStruct.name, field.name));
                break;
            case TypeKind::UNKNOWN:
                // TODO: pointers
                throw UnImplementedException(
                    std::string("Structs don't support fields of type: " + field.type.typeName())
                );

            default:
                std::string message = "Missing case for this TypeKind in switch";
                LOG_S(ERROR) << message;
                throw NoSuchTypeException(message);
        }

        if (!dirtyInitializedField && sizeOfFieldToInitUnion < fieldLen &&
            curStruct.subType == types::SubType::Union) {
            fieldIndexToInitUnion = fieldIndex;
            sizeOfFieldToInitUnion = fieldLen;
            isInitializedStruct = true;
            dirtyInitializedStruct = false;
        }
        if (curStruct.subType == types::SubType::Struct) {
            dirtyInitializedStruct |= dirtyInitializedField;
            isInitializedStruct &= isInitializedField;
        }
        prevFieldEndOffset = fieldEndOffset;
        ++fieldIndex;
    }

    std::optional<std::string> entryValue;
    if (!isInitializedStruct && !curStruct.name.empty() && !anonymousField) {
        // init by memory copy
        entryValue = PrinterUtils::convertBytesToStruct(
            curStruct.name,
            arrayView(byteArray, lazyPointersArray,
                      types::Type::createSimpleTypeFromName("utbot_byte"),
                      curStruct.size,
                      offsetInBits, usage)->getEntryValue(nullptr));
        isInitializedStruct = true;
        dirtyInitializedStruct = false;
    }
    if (!isInitializedStruct) {
        dirtyInitializedStruct = false;
    }
    return std::make_shared<StructValueView>(curStruct, subViews, entryValue,
                                             anonymousField, isInitializedStruct, dirtyInitializedStruct, fieldIndexToInitUnion);
}

std::string KTestObjectParser::primitiveCharView(const types::Type &type, std::string value) {
    if (types::TypesHandler::isCharacterType(type)) {
        return "\'" + StringUtils::charCodeToLiteral(std::stoi(value)) + "\'";
    }
    return value;
}

std::string KTestObjectParser::primitiveBoolView(const std::string &value) {
    if (value != "0") {
        return "true";
    }
    return "false";
}

std::string readBytesAsValueForType(const std::vector<char> &byteArray,
                                    const std::string &typeName,
                                    size_t offsetInBits,
                                    size_t lenInBits) {
    if (typeName == "utbot_byte") {
        //we use different name to not trigger char processing
        return readBytesAsValue<char>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "short") {
        return readBytesAsValue<short>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "int") {
        return readBytesAsValue<int>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "long") {
        return readBytesAsValue<long>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "long long") {
        return readBytesAsValue<long long>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "unsigned short") {
        return readBytesAsValue<unsigned short>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "unsigned int") {
        return readBytesAsValue<unsigned int>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "unsigned long") {
        return readBytesAsValue<unsigned long>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "unsigned long long") {
        return readBytesAsValue<unsigned long long>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "char") {
        return readBytesAsValue<char>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "signed char") {
        return readBytesAsValue<signed char>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "unsigned char") {
        return readBytesAsValue<unsigned char>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "bool" || typeName == "_Bool") {
        return readBytesAsValue<bool>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "float") {
        return readBytesAsValue<float>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "double") {
        return readBytesAsValue<double>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "long double") {
        return readBytesAsValue<long double>(byteArray, offsetInBits, lenInBits);
    }
    if (typeName == "std::uintptr_t" || typeName == "uintptr_t") {
        return readBytesAsValue<std::uintptr_t>(byteArray, offsetInBits, lenInBits);
    }
    return "";
}

namespace { //Predicate utilities.
    //Those should never abort as we do not accept such data on client side.
    template <typename T>
    bool compareSimpleValues(const std::string &cmp, T a, T b) {
        if (cmp == "==") {
            return a == b;
        } else if (cmp == "!=") {
            return a != b;
        } else if (cmp == "<") {
            return a < b;
        } else if (cmp == ">") {
            return a > b;
        } else if (cmp == "<=") {
            return a <= b;
        } else if (cmp == ">=") {
            return a >= b;
        } else {
            ABORT_F("Wrong predicate: %s", cmp.c_str());
        }
     }

    bool predicateMatch(const std::string &value, const LineInfo::PredicateInfo &info) {
        switch (info.type) {
            case testsgen::CHAR:
                return compareSimpleValues(info.predicate, value, "\'" + info.returnValue + "\'");
            case testsgen::STRING:
                return compareSimpleValues(info.predicate, value, "\"" + info.returnValue + "\"");
            case testsgen::INT8_T:
            case testsgen::INT16_T:
            case testsgen::INT32_T:
            case testsgen::INT64_T:
                return compareSimpleValues(info.predicate, StringUtils::stot<long long>(value),
                                           StringUtils::stot<long long>(info.returnValue));
            case testsgen::UINT8_T:
            case testsgen::UINT16_T:
            case testsgen::UINT32_T:
            case testsgen::UINT64_T:
                return compareSimpleValues(info.predicate, StringUtils::stot<unsigned long long>(value),
                                           StringUtils::stot<unsigned long long>(info.returnValue));
            case testsgen::BOOL:
                return compareSimpleValues(info.predicate, StringUtils::stot<bool>(value),
                                           StringUtils::stot<bool>(info.returnValue));
            case testsgen::FLOAT:
                return compareSimpleValues(info.predicate, StringUtils::stot<float>(value),
                                           StringUtils::stot<float>(info.returnValue));
            default:
                ABORT_F("Unsupported ValidationType: %s", ValidationType_Name(info.type).c_str());
        }
    }
}

void KTestObjectParser::parseKTest(const MethodKtests &batch,
                                   tests::Tests &tests,
                                   const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
                                   bool filterByLineFlag,
                                   const std::shared_ptr<LineInfo> &lineInfo) {
    LOG_SCOPE_FUNCTION(DEBUG);
    sourceFilePath = tests.sourceFilePath;
    for (auto &[testMethod, testCases] : batch) {
        auto it = tests.methods.find<std::string, tests::Tests::MethodDescriptionToStringEqual>(
            testMethod.methodName);
        LOG_S(DEBUG) << "Parse klee for method: " << testMethod.methodName;
        parseTestCases(testCases, filterByLineFlag, it.value(), methodNameToReturnTypeMap, lineInfo);
    }
}

static std::string getSuiteName(const UTBotKTest::Status &status,
                                const std::shared_ptr<LineInfo> &lineInfo) {
    bool forAssert = lineInfo != nullptr && lineInfo->forAssert;
    if (status == UTBotKTest::Status::FAILED || forAssert) {
        return Tests::ERROR_SUITE_NAME;
    }
    return Tests::DEFAULT_SUITE_NAME;
}

size_t KTestObjectParser::findFieldIndex(const StructInfo &structInfo, size_t offsetInBits) const {
    size_t indField = std::upper_bound(structInfo.fields.begin(), structInfo.fields.end(), offsetInBits, [] (int offset, const Field &field) {
        return offset < field.offset;
    }) - structInfo.fields.begin();
    if (indField == 0) {
        std::string message = "Wrong offset";
        LOG_S(ERROR) << message;
        throw IncorrectIndexException(message);
    }
    return indField - 1;
}

void KTestObjectParser::addToOrder(const std::vector<UTBotKTestObject> &objects,
                                   const std::string &paramName,
                                   const types::Type &paramType,
                                   Tests::TestCaseParamValue &paramValue,
                                   std::vector<bool> &visited,
                                   std::vector<PointerUsage> &usages,
                                   std::queue<JsonIndAndParam> &order) {
    auto it = std::find_if(objects.begin(), objects.end(),
                     [paramName](const UTBotKTestObject &obj) { return obj.name == paramName; });
    if (it != objects.end()) {
        size_t jsonInd = it - objects.begin();
        visited[jsonInd] = true;
        usages[jsonInd] = types::PointerUsage::PARAMETER;
        Tests::MethodParam param = { paramType.isObjectPointer() && !paramType.isPointerToPointer()
                                         ? paramType.baseTypeObj()
                                         : paramType,
                                    paramName, std::nullopt };
        order.emplace(jsonInd, param, paramValue);
        return;
    }
    std::string message = "Don't find object " + paramName + " in objects array";
    LOG_S(WARNING) << message;
}

bool KTestObjectParser::pointToStruct(const types::Type &pointerType,
                                      const UTBotKTestObject &goal) const {
    // In different situations we may point on the whole struct or on the field with assignment 0
    size_t fieldSizeInBits = typesHandler.typeSize(pointerType.baseTypeObj(1));
    size_t pointerVarSizeInBytes = goal.bytes.size();
    return SizeUtils::bytesToBits(pointerVarSizeInBytes) == fieldSizeInBits;
}

void KTestObjectParser::assignTypeUnnamedVar(
    Tests::MethodTestCase &testCase,
    const Tests::MethodDescription &methodDescription,
    std::vector<std::optional<Tests::TypeAndVarName>> &objects,
    std::vector<PointerUsage> &usages) {
    std::queue<JsonIndAndParam> order;
    std::vector<bool> visited(testCase.objects.size(), false);
    for (size_t paramInd = 0; paramInd < testCase.paramValues.size(); paramInd++) {
        addToOrder(testCase.objects, methodDescription.params[paramInd].name,
                   methodDescription.params[paramInd].type, testCase.paramValues[paramInd], visited,
                   usages, order);
    }
    addToOrder(testCase.objects, KleeUtils::RESULT_VARIABLE_NAME, methodDescription.returnType,
               testCase.returnValue, visited, usages, order);

    while (!order.empty()) {
        auto curType = order.front();
        order.pop();
        std::string name = testCase.objects[curType.jsonInd].name;
        types::Type paramType = curType.param.type;
        objects[curType.jsonInd] = { paramType, name };

        if (testCase.objects[curType.jsonInd].is_lazy) {
            if (types::TypesHandler::baseTypeIsVoid(paramType)) {
                throw UnImplementedException("Lazy variable has baseType=void");
            }

            usages[curType.jsonInd] = types::PointerUsage::LAZY;
            std::vector<char> byteValue = testCase.objects[curType.jsonInd].bytes;
            Tests::TypeAndVarName typeAndVarName{ paramType, name };
            std::shared_ptr<AbstractValueView> testParamView = testParameterView(
                { name, byteValue, testCase.objects[curType.jsonInd].pointers }, typeAndVarName,
                PointerUsage::LAZY, testCase.objects, testCase.lazyReferences,
                methodDescription);
            LOG_S(MAX) << "Fetch lazy object: " << name << " = "
                       << testParamView->getEntryValue(nullptr);
            curType.paramValue.lazyParams.emplace_back(paramType, name, std::nullopt);
            curType.paramValue.lazyValues.emplace_back(name, std::nullopt, testParamView);
        }

        for (auto const &[offset, indObj, indexOffset] : testCase.objects[curType.jsonInd].pointers) {
            if (indexOffset != 0) {
                continue;
            }
            Tests::TypeAndVarName typeAndName = { paramType, "" };
            size_t offsetInStruct =
                getOffsetInStruct(typeAndName, SizeUtils::bytesToBits(offset), usages[indObj]);
            types::Type fieldType = traverseLazyInStruct(typeAndName.type, offsetInStruct).type;
            if (!pointToStruct(fieldType, testCase.objects[indObj])) {
                continue;
            }
            if (!visited[indObj]) {
                Tests::MethodParam param = {fieldType.baseTypeObj(1), "", std::nullopt};
                order.emplace(indObj, param, curType.paramValue);
                visited[indObj] = true;
                usages[indObj] = types::PointerUsage::PARAMETER;
            }
        }
    }
}

Tests::TypeAndVarName KTestObjectParser::traverseLazyInStruct(const types::Type &curVarType,
                                                              size_t offsetInBits,
                                                              const std::string &curVarName) const {
    switch (typesHandler.getTypeKind(curVarType)) {
        case TypeKind::STRUCT_LIKE: {
            const types::StructInfo &structInfo = typesHandler.getStructInfo(curVarType);
            size_t indField = findFieldIndex(structInfo, offsetInBits);
            const types::Field &next = structInfo.fields[indField];
            return traverseLazyInStruct(next.type, offsetInBits - next.offset,
                                        PrinterUtils::getFieldAccess(curVarName, next));
        }
        case TypeKind::OBJECT_POINTER: {
            LOG_IF_S(ERROR, offsetInBits != 0) << "Offset not zero" << offsetInBits;
            return { curVarType, curVarName };
        }
        case TypeKind::PRIMITIVE: {
            return { curVarType, curVarName };
        }
        case TypeKind::ENUM:
        case TypeKind::FUNCTION_POINTER:
        case TypeKind::ARRAY:
        case TypeKind::UNKNOWN:
        default: {
            std::string message =
                "Unsupported type in lazy initialization BFS: " + curVarType.typeName();
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
    }
}

size_t KTestObjectParser::getOffsetInStruct(Tests::TypeAndVarName &objTypeAndName,
                                            size_t offsetInBits,
                                            types::PointerUsage usage) const {
    if (!objTypeAndName.type.isPointerToPointer() || usage != types::PointerUsage::PARAMETER) {
        return offsetInBits;
    }
    std::vector<size_t> sizes = objTypeAndName.type.arraysSizes(usage);
    objTypeAndName.type = objTypeAndName.type.baseTypeObj();
    size_t sizeInBits = typesHandler.typeSize(objTypeAndName.type);
    size_t offset = offsetInBits / sizeInBits;
    PrinterUtils::appendIndicesToVarName(objTypeAndName.varName, sizes, offset);
    offsetInBits %= sizeInBits;
    return offsetInBits;
}

void KTestObjectParser::assignTypeStubVar(Tests::MethodTestCase &testCase,
                                          const Tests::MethodDescription &methodDescription) {
    for (auto const &obj : testCase.objects) {
        std::optional<std::shared_ptr<FunctionInfo>> maybeFunctionInfo =
            methodDescription.stubsStorage->getFunctionPointerByKTestObjectName(obj.name);
        if (maybeFunctionInfo.has_value()) {
            types::Type stubType = types::Type::createArray(maybeFunctionInfo.value()->returnType);
            std::shared_ptr<AbstractValueView> stubView =
                testParameterView({ obj.name, obj.bytes, obj.pointers }, { stubType, obj.name },
                                  PointerUsage::PARAMETER, testCase.objects,
                                  testCase.lazyReferences, methodDescription);
            testCase.stubParamValues.emplace_back(obj.name, 0, stubView);
            testCase.stubParamTypes.emplace_back(stubType, obj.name, std::nullopt);
        }
    }
}

void KTestObjectParser::assignAllLazyPointers(
    Tests::MethodTestCase &testCase,
    const std::vector<std::optional<Tests::TypeAndVarName>> &objTypeAndName,
    const std::vector<PointerUsage> &usages) const {
    for (size_t ind = 0; ind < testCase.objects.size(); ind++) {
        const auto &object = testCase.objects[ind];
        if (!objTypeAndName[ind].has_value()) {
            continue;
        }
        for (const auto &pointer : object.pointers) {
            Tests::TypeAndVarName typeAndName = objTypeAndName[ind].value();
            size_t offset = getOffsetInStruct(typeAndName,
                                              SizeUtils::bytesToBits(pointer.offset),
                                              usages[ind]);
            Tests::TypeAndVarName fromPtr =
                traverseLazyInStruct(typeAndName.type, offset, typeAndName.varName);
            if (!objTypeAndName[pointer.index].has_value()) {
                continue;
            }

            std::string toPtrName;
            Tests::TypeAndVarName pointerTypeAndName = objTypeAndName[pointer.index].value();
            size_t indexOffset = getOffsetInStruct(pointerTypeAndName,
                                                   SizeUtils::bytesToBits(pointer.indexOffset),
                                                   usages[pointer.index]);
            if (indexOffset == 0 &&
                pointToStruct(fromPtr.type, testCase.objects[pointer.index])) {
                toPtrName = pointerTypeAndName.varName;
            } else {
                toPtrName = traverseLazyInStruct(pointerTypeAndName.type, indexOffset,
                                                 pointerTypeAndName.varName).varName;
            }

            testCase.lazyReferences.emplace_back(
                fromPtr.varName, toPtrName,
                PrinterUtils::initializePointerToVar(fromPtr.type.baseType(), toPtrName,
                                                     fromPtr.type.getDimension()));
        }
    }
}

void KTestObjectParser::parseTestCases(const UTBotKTestList &cases,
                                       bool filterByLineFlag,
                                       Tests::MethodDescription &methodDescription,
                                       const std::unordered_map<std::string, types::Type>& methodNameToReturnTypeMap,
                                       const std::shared_ptr<LineInfo> &lineInfo) {
    /* Replace the return type for predicate scenario
     * to treat strings in specific way. This is done to retrieve
     * correct value from KTests and print the test.
     */
    if (lineInfo && lineInfo->predicateInfo.has_value() && lineInfo->predicateInfo->type == testsgen::STRING) {
        methodDescription.returnType = types::Type::CStringType();
    }
    int caseCounter = 0;

    int testIndex = 0;
    for (const auto &case_ : cases) {
        try {
            std::stringstream traceStream;
            traceStream << "Test case #" << (++caseCounter) << ":\n";
            std::string suiteName = getSuiteName(case_.status, lineInfo);
            Tests::MethodTestCase testCase{testIndex, suiteName};
            std::vector<Tests::TestCaseParamValue> paramValues;

            Tests::TestCaseDescription testCaseDescription = parseTestCaseParameters(case_, methodDescription,
                                                                                     methodNameToReturnTypeMap,
                                                                                     traceStream);
            size_t size = case_.objects.size();
            bool isVoidOrFPointer = types::TypesHandler::skipTypeInReturn(methodDescription.returnType);
            if ((isVoidOrFPointer && size > 0) || (!isVoidOrFPointer && size > 1) ||
                methodDescription.params.empty()) {
                std::swap(testCase.paramValues, testCaseDescription.funcParamValues);
            } else {
                // if all the data characters are not printable the case is skipped
                continue;
            }
            std::swap(testCase.classPreValues, testCaseDescription.classPreValues);
            std::swap(testCase.classPostValues, testCaseDescription.classPostValues);
            std::swap(testCase.globalPreValues, testCaseDescription.globalPreValues);
            std::swap(testCase.globalPostValues, testCaseDescription.globalPostValues);
            std::swap(testCase.paramPostValues, testCaseDescription.paramPostValues);
            std::swap(testCase.stubValuesTypes, testCaseDescription.stubValuesTypes);
            std::swap(testCase.stubValues, testCaseDescription.stubValues);
            std::swap(testCase.stdinValue, testCaseDescription.stdinValue);
            std::swap(testCase.filesValues, testCaseDescription.filesValues);
            std::swap(testCase.objects, testCaseDescription.objects);
            std::swap(testCase.lazyReferences, testCaseDescription.lazyReferences);

            testCase.errorDescriptors = case_.errorDescriptors;

            testCase.errorInfo = testCaseDescription.errorInfo;
            if (filterByLineFlag) {
                auto view = testCaseDescription.kleePathFlagSymbolicValue.view;
                if (!view || view->getEntryValue(nullptr) != "1") {
                    continue;
                }
            }
            auto const &predicateInfo = lineInfo ? lineInfo->predicateInfo : std::nullopt;
            if (predicateInfo.has_value() &&
                !predicateMatch(testCaseDescription.returnValue.view->getEntryValue(nullptr), predicateInfo.value())) {
                continue;
            }

            if (predicateInfo.has_value() && predicateInfo->type != testsgen::STRING) {
                testCase.returnValue.view = std::make_shared<PrimitiveValueView>(
                        PrinterUtils::wrapUserValue(predicateInfo->type, predicateInfo->returnValue));
            } else {
                testCase.returnValue.view = testCaseDescription.returnValue.view;
            }

            if (methodDescription.returnType.isObjectPointer() && !methodDescription.returnType.maybeArray
                && testCaseDescription.functionReturnNotNullValue.view &&
                testCaseDescription.functionReturnNotNullValue.view->getEntryValue(nullptr) == "0") {
                testCase.returnValue.view = std::make_shared<PrimitiveValueView>(PrinterUtils::C_NULL);
            }
            traceStream << "\treturn: " << testCase.returnValue.view->getEntryValue(nullptr);
            LOG_S(MAX) << traceStream.str();

            std::vector<std::optional<Tests::TypeAndVarName>> objectsValues(testCase.objects.size());
            std::vector<PointerUsage> usages(testCase.objects.size());
            assignTypeUnnamedVar(testCase, methodDescription, objectsValues, usages);
            assignTypeStubVar(testCase, methodDescription);
            assignAllLazyPointers(testCase, objectsValues, usages);

            methodDescription.testCases.push_back(testCase);
            methodDescription.suiteTestCases[testCase.suiteName].push_back(testCase.testIndex);
            ++testIndex;
        } catch (const UnImplementedException &e) {
            LOG_S(WARNING) << "Skipping test case: " << e.what();
        } catch (const NoSuchTypeException &e) {
            LOG_S(WARNING) << "Skipping test case: " << e.what();
        }
    }
}

std::vector<KTestObjectParser::RawKleeParam>::const_iterator
KTestObjectParser::getKleeParam(const std::vector<RawKleeParam> &rawKleeParams, const std::string name) {
    return std::find_if(rawKleeParams.begin(), rawKleeParams.end(),
                        [&](const RawKleeParam &param) { return param.paramName == name; });
}

KTestObjectParser::RawKleeParam
KTestObjectParser::getKleeParamOrThrow(const std::vector<RawKleeParam> &rawKleeParams,
                                       const std::string &name) {
    const auto kleeParam = getKleeParam(rawKleeParams, name);
    if (kleeParam == rawKleeParams.end()) {
        std::string message = "Parameter \'" + name + "\' not found.";
        LOG_S(ERROR) << message;
        throw UnImplementedException(message);
    }

    return *kleeParam;
}

Tests::TestCaseDescription
KTestObjectParser::parseTestCaseParameters(const UTBotKTest &testCases,
                                           Tests::MethodDescription &methodDescription,
                                           const std::unordered_map<std::string, types::Type>& methodNameToReturnTypeMap,
                                           std::stringstream &traceStream) {
    return parseTestCaseParams(testCases, methodDescription, methodNameToReturnTypeMap, traceStream);
}

Tests::TestCaseDescription KTestObjectParser::parseTestCaseParams(
    const UTBotKTest &ktest,
    const Tests::MethodDescription &methodDescription,
    const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
    const std::stringstream &traceStream) {
    std::vector<RawKleeParam> rawKleeParams;
    for (auto const &param : ktest.objects) {
        rawKleeParams.emplace_back(param.name, param.bytes, param.pointers);
    }

    Tests::TestCaseDescription testCaseDescription;
    testCaseDescription.objects = ktest.objects;
    testCaseDescription.errorInfo = ktest.errorInfo;

    int cnt = 0;
    for (auto &obj : testCaseDescription.objects) {
        if (obj.name != LAZYNAME) {
            continue;
        }
        obj.name = PrinterUtils::generateNewVar(++cnt);
    }

    const RawKleeParam emptyKleeParam = { "", {}, {} };

    if (methodDescription.isClassMethod()) {
        auto methodParam = methodDescription.classObj.value();
        std::shared_ptr<AbstractValueView> testParamView;
        getTestParamView(methodDescription, rawKleeParams, emptyKleeParam, testCaseDescription,
                         methodParam, testParamView);
        testCaseDescription.classPreValues = { methodParam.name, methodParam.alignment,
                                               testParamView };
        processClassPostValue(testCaseDescription, methodParam, rawKleeParams);
    }

    for (auto &methodParam : methodDescription.params) {
        std::shared_ptr<AbstractValueView> testParamView;
        if (!methodParam.type.isFilePointer()) {
            getTestParamView(methodDescription, rawKleeParams, emptyKleeParam, testCaseDescription,
                             methodParam, testParamView);
        } else {
            testParamView = std::shared_ptr<AbstractValueView>(new JustValueView("FILE_PTR"));
        }
        testCaseDescription.funcParamValues.emplace_back(methodParam.name, methodParam.alignment,
                                                         testParamView);

        if (methodParam.isChangeable()) {
            processParamPostValue(testCaseDescription, methodParam, rawKleeParams);
        }
    }
    for (const auto &globalParam : methodDescription.globalParams) {
        processGlobalParamPreValue(testCaseDescription, globalParam, rawKleeParams);
        processGlobalParamPostValue(testCaseDescription, globalParam, rawKleeParams);
    }

    if (Paths::getSourceLanguage(sourceFilePath) == utbot::Language::C) {
        processSymbolicStdin(testCaseDescription, rawKleeParams);
        processSymbolicFiles(testCaseDescription, rawKleeParams);
    }

    processStubParamValue(testCaseDescription, methodNameToReturnTypeMap, rawKleeParams);
    if (!types::TypesHandler::skipTypeInReturn(methodDescription.returnType)) {
        const auto kleeResParam =
            getKleeParamOrThrow(rawKleeParams, KleeUtils::RESULT_VARIABLE_NAME);
        auto paramType = methodDescription.returnType.maybeReturnArray()
                             ? methodDescription.returnType
                             : methodDescription.returnType.baseTypeObj();
        const Tests::TypeAndVarName returnParam = { paramType, KleeUtils::RESULT_VARIABLE_NAME };
        const auto testReturnView = testParameterView(
            kleeResParam, returnParam, PointerUsage::RETURN, testCaseDescription.objects,
            testCaseDescription.lazyReferences, methodDescription);
        testCaseDescription.returnValue = {
            KleeUtils::RESULT_VARIABLE_NAME,
            types::TypesHandler::isObjectPointerType(methodDescription.returnType), testReturnView
        };
    } else {
        testCaseDescription.returnValue = { KleeUtils::RESULT_VARIABLE_NAME, false,
                                            std::make_shared<VoidValueView>() };
    }

    const auto kleePathFlagIterator = getKleeParam(rawKleeParams, KLEE_PATH_FLAG);
    const auto kleePathFlagSymbolicIterator = getKleeParam(rawKleeParams, KLEE_PATH_FLAG_SYMBOLIC);
    if (kleePathFlagSymbolicIterator != rawKleeParams.end()) {
        const Tests::TypeAndVarName kleePathParam = { types::Type::intType(),
                                                      KLEE_PATH_FLAG_SYMBOLIC };
        const auto kleePathFlagSymbolicView = testParameterView(
            *kleePathFlagSymbolicIterator, kleePathParam, types::PointerUsage::PARAMETER,
            testCaseDescription.objects, testCaseDescription.lazyReferences);
        testCaseDescription.kleePathFlagSymbolicValue = { KLEE_PATH_FLAG_SYMBOLIC, false,
                                                          kleePathFlagSymbolicView };
    }
    const auto functionReturnNotNullIterator =
        getKleeParam(rawKleeParams, KleeUtils::NOT_NULL_VARIABLE_NAME);
    if (functionReturnNotNullIterator != rawKleeParams.end()) {
        const Tests::TypeAndVarName functionReturnNotNull = { types::Type::intType(),
                                                              KleeUtils::NOT_NULL_VARIABLE_NAME };
        const auto functionReturnNotNullView = testParameterView(
            *functionReturnNotNullIterator, functionReturnNotNull, types::PointerUsage::PARAMETER,
            testCaseDescription.objects, testCaseDescription.lazyReferences);
        testCaseDescription.functionReturnNotNullValue = { KleeUtils::NOT_NULL_VARIABLE_NAME, false,
                                                           functionReturnNotNullView };
    }
    return testCaseDescription;
}

void KTestObjectParser::getTestParamView(const Tests::MethodDescription &methodDescription,
                                         const std::vector<RawKleeParam> &rawKleeParams,
                                         const KTestObjectParser::RawKleeParam &emptyKleeParam,
                                         Tests::TestCaseDescription &testCaseDescription,
                                         const Tests::MethodParam &methodParam,
                                         std::shared_ptr<AbstractValueView> &testParamView) {
    const auto usage = types::PointerUsage::PARAMETER;
    types::Type paramType = methodParam.type.arrayCloneMultiDim(usage);
    auto type = typesHandler.getReturnTypeToCheck(paramType);

    if (CollectionUtils::containsKey(methodDescription.functionPointers, methodParam.name)) {
        testParamView = testParameterView(emptyKleeParam, { type, methodParam.name },
                                          PointerUsage::PARAMETER, testCaseDescription.objects,
                                          testCaseDescription.lazyReferences, methodDescription);
    } else {
        const auto kleeParam = getKleeParamOrThrow(rawKleeParams, methodParam.name);
        testParamView = testParameterView(kleeParam, { type, methodParam.name },
                                          PointerUsage::PARAMETER, testCaseDescription.objects,
                                          testCaseDescription.lazyReferences, methodDescription);
    }
}

void KTestObjectParser::processGlobalParamPreValue(Tests::TestCaseDescription &testCaseDescription,
                                                   const Tests::MethodParam &globalParam,
                                                   std::vector<RawKleeParam> &rawKleeParams) {
    std::string kleeParamName = globalParam.name;
    auto kleeParam = getKleeParamOrThrow(rawKleeParams, kleeParamName);
    auto testParamView = testParameterView(
        kleeParam, { globalParam.type, globalParam.name }, types::PointerUsage::PARAMETER,
        testCaseDescription.objects, testCaseDescription.lazyReferences);
    testCaseDescription.globalPreValues.emplace_back(globalParam.name, globalParam.alignment,
                                                     testParamView);
}

void KTestObjectParser::processSymbolicStdin(Tests::TestCaseDescription &testCaseDescription,
                                             const std::vector<RawKleeParam> &rawKleeParams) {
    auto &&read = getKleeParamOrThrow(rawKleeParams, "stdin-read");
    std::string &&view =
        testParameterView(read, { types::Type::longlongType(), "stdin-read" },
                          types::PointerUsage::PARAMETER, testCaseDescription.objects,
                          testCaseDescription.lazyReferences)
            ->getEntryValue(nullptr);
    if (view == "0LL") {
        return;
    } else {
        long long usedStdinBytesCount = std::stoll(view);
        if (usedStdinBytesCount > types::Type::symInputSize) {
            std::string message = ".ktest has malformed stdin data";
            LOG_S(ERROR) << message;
            throw UnImplementedException(message);
        }
        auto &&stdinBuffer = getKleeParamOrThrow(rawKleeParams, "stdin");
        auto &&testParamView = stringLiteralView(stdinBuffer.rawData, usedStdinBytesCount);
        testCaseDescription.stdinValue = Tests::TestCaseParamValue(types::Type::getStdinParamName(),
                                                                   std::nullopt, testParamView);
    }
}

void KTestObjectParser::processSymbolicFiles(Tests::TestCaseDescription &testCaseDescription,
                                             const std::vector<RawKleeParam> &rawKleeParams) {
    std::vector<Tests::FileInfo> filesValues(types::Type::symFilesCount);
    int fileIndex = 0;
    for (char fileName = 'A'; fileName < 'A' + types::Type::symFilesCount;
         fileName++, fileIndex++) {
        std::string readBytesName = PrinterUtils::getFileReadBytesParamKTestJSON(fileName);
        auto &&readBytes = getKleeParamOrThrow(rawKleeParams, readBytesName);
        filesValues[fileIndex].readBytes =
            std::stoi(testParameterView(readBytes, { types::Type::longlongType(), readBytesName },
                                        types::PointerUsage::PARAMETER, testCaseDescription.objects,
                                        testCaseDescription.lazyReferences)
                          ->getEntryValue(nullptr));

        std::string writeBytesName = PrinterUtils::getFileWriteBytesParamKTestJSON(fileName);
        auto &&writeBytes = getKleeParamOrThrow(rawKleeParams, writeBytesName);
        filesValues[fileIndex].writeBytes =
            std::stoi(testParameterView(writeBytes, { types::Type::longlongType(), writeBytesName },
                                        types::PointerUsage::PARAMETER, testCaseDescription.objects,
                                        testCaseDescription.lazyReferences)
                          ->getEntryValue(nullptr));

        auto &&fileBuffer =
            getKleeParamOrThrow(rawKleeParams, PrinterUtils::getFileParamKTestJSON(fileName));
        filesValues[fileIndex].data =
            stringLiteralView(fileBuffer.rawData, filesValues[fileIndex].readBytes)
                ->getEntryValue(nullptr);
    }
    testCaseDescription.filesValues = filesValues;
}

void KTestObjectParser::processGlobalParamPostValue(Tests::TestCaseDescription &testCaseDescription,
                                                    const Tests::MethodParam &globalParam,
                                                    std::vector<RawKleeParam> &rawKleeParams) {
    auto symbolicVariable = KleeUtils::postSymbolicVariable(globalParam.name);
    auto kleeParam = getKleeParamOrThrow(rawKleeParams, symbolicVariable);
    auto type = typesHandler.getReturnTypeToCheck(globalParam.type);
    Tests::TypeAndVarName typeAndVarName{ type, globalParam.name };
    auto testParamView =
        testParameterView(kleeParam, typeAndVarName, types::PointerUsage::PARAMETER,
                          testCaseDescription.objects, testCaseDescription.lazyReferences);
    testCaseDescription.globalPostValues.emplace_back(globalParam.name, globalParam.alignment,
                                                      testParamView);
}

void KTestObjectParser::processClassPostValue(Tests::TestCaseDescription &testCaseDescription,
                                              const Tests::MethodParam &param,
                                              std::vector<RawKleeParam> &rawKleeParams) {
    const auto usage = types::PointerUsage::PARAMETER;
    auto symbolicVariable = KleeUtils::postSymbolicVariable(param.name);
    auto kleeParam = getKleeParamOrThrow(rawKleeParams, symbolicVariable);
    types::Type paramType = param.type.arrayCloneMultiDim(usage);
    auto type = typesHandler.getReturnTypeToCheck(paramType);
    Tests::TypeAndVarName typeAndVarName{ type, param.name };
    auto testParamView =
        testParameterView(kleeParam, typeAndVarName, usage, testCaseDescription.objects,
                          testCaseDescription.lazyReferences);
    testCaseDescription.classPostValues = { param.name, param.alignment, testParamView };
}

void KTestObjectParser::processParamPostValue(Tests::TestCaseDescription &testCaseDescription,
                                              const Tests::MethodParam &param,
                                              std::vector<RawKleeParam> &rawKleeParams) {
    const auto usage = types::PointerUsage::PARAMETER;
    auto symbolicVariable = KleeUtils::postSymbolicVariable(param.name);
    auto kleeParam = getKleeParamOrThrow(rawKleeParams, symbolicVariable);
    types::Type paramType = param.type.arrayCloneMultiDim(usage);
    auto type = typesHandler.getReturnTypeToCheck(paramType);
    Tests::TypeAndVarName typeAndVarName{ type, param.name };
    auto testParamView =
        testParameterView(kleeParam, typeAndVarName, usage, testCaseDescription.objects,
                          testCaseDescription.lazyReferences);
    testCaseDescription.paramPostValues.emplace_back(param.name, param.alignment, testParamView);
}

void KTestObjectParser::processStubParamValue(
    Tests::TestCaseDescription &testCaseDescription,
    const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
    std::vector<RawKleeParam> &rawKleeParams) {
    for (const auto &kleeParam : rawKleeParams) {
        if (StringUtils::endsWith(kleeParam.paramName, PrinterUtils::KLEE_SYMBOLIC_SUFFIX)) {
            std::string methodName = kleeParam.paramName.substr(
                0, kleeParam.paramName.size() - PrinterUtils::KLEE_SYMBOLIC_SUFFIX.size());
            if (!CollectionUtils::containsKey(methodNameToReturnTypeMap, methodName)) {
                LOG_S(WARNING) << "Method name \"" << methodName << "\" was not fetched, skipping";
                continue;
            }
            auto type = typesHandler.getReturnTypeToCheck(methodNameToReturnTypeMap.at(methodName));
            Tests::TypeAndVarName typeAndVarName{ type, kleeParam.paramName };
            auto testParamView =
                testParameterView(kleeParam, typeAndVarName, types::PointerUsage::PARAMETER,
                                  testCaseDescription.objects, testCaseDescription.lazyReferences);
            testCaseDescription.stubValues.emplace_back(kleeParam.paramName, 0, testParamView);
            testCaseDescription.stubValuesTypes.emplace_back(type, kleeParam.paramName, 0);
        }
    }
}

std::shared_ptr<AbstractValueView> KTestObjectParser::testParameterView(
    const KTestObjectParser::RawKleeParam &kleeParam,
    const Tests::TypeAndVarName &param,
    PointerUsage usage,
    const std::vector<UTBotKTestObject> &objects,
    std::vector<InitReference> &initReferences,
    const std::optional<const Tests::MethodDescription> &testingMethod) {
    const auto &rawData = kleeParam.rawData;
    const auto &paramType = param.type;
    switch (typesHandler.getTypeKind(paramType)) {
        case TypeKind::STRUCT_LIKE:
            return structView(rawData, kleeParam.pointers, typesHandler.getStructInfo(paramType), 0,
                              usage, testingMethod, false, param.varName, objects, initReferences);
        case TypeKind::ENUM:
            return enumView(rawData, typesHandler.getEnumInfo(paramType), 0,
                            SizeUtils::bytesToBits(rawData.size()));
        case TypeKind::PRIMITIVE:
            return primitiveView(rawData, paramType.baseTypeObj(), 0,
                                 SizeUtils::bytesToBits(rawData.size()));
        case TypeKind::OBJECT_POINTER:
            if (usage == types::PointerUsage::LAZY) {
                std::string res =
                    readBytesAsValueForType(rawData, PointerWidthType, 0, PointerWidthSizeInBits);
                return getLazyPointerView(objects, initReferences, param.varName, res, paramType,
                                          !kleeParam.pointers.empty());
            } else if (types::TypesHandler::isCStringType(paramType)) {
                return stringLiteralView(rawData);
            } else if (paramType.kinds().size() > 2) {
                return multiArrayView(rawData, kleeParam.pointers, paramType,
                                      SizeUtils::bytesToBits(rawData.size()), 0, usage);
            } else {
                return arrayView(rawData, kleeParam.pointers, paramType.baseTypeObj(),
                                 SizeUtils::bytesToBits(rawData.size()), 0, usage);
            }
        case TypeKind::FUNCTION_POINTER:
            if (!testingMethod.has_value()) {
                return functionPointerView(std::nullopt, "", param.varName);
            }
            return functionPointerView(testingMethod->getClassTypeName(), testingMethod->name,
                                       param.varName);
        case TypeKind::ARRAY:
            if (paramType.kinds().size() > 2) {
                return multiArrayView(rawData, kleeParam.pointers, paramType,
                                      SizeUtils::bytesToBits(rawData.size()), 0, usage);
            } else {
                return arrayView(rawData, kleeParam.pointers, paramType.baseTypeObj(),
                                 SizeUtils::bytesToBits(rawData.size()), 0, usage);
            }
        case TypeKind::UNKNOWN:
            throw UnImplementedException("No such type");
        default: {
            std::string message = "Missing case for this TypeKind in switch";
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
    }
}

std::shared_ptr<AbstractValueView>
KTestObjectParser::getLazyPointerView(const std::vector<UTBotKTestObject> &objects,
                                      std::vector<InitReference> &initReferences,
                                      const std::string &name,
                                      std::string res,
                                      const Type &paramType,
                                      bool lazyPointer) const {
    size_t ptr = std::stoull(res);
    auto ptr_element =
        std::find_if(objects.begin(), objects.end(),
                     [ptr](const UTBotKTestObject &object) { return object.address == ptr; });
    if (!lazyPointer && ptr_element != objects.end()) {
            initReferences.emplace_back(
                name, ptr_element->name,
                PrinterUtils::initializePointerToVar(paramType.baseType(), ptr_element->name,
                                                     paramType.getDimension()));
    }
    if (lazyPointer || ptr_element != objects.end()) {
            res = PrinterUtils::C_NULL;
    }
    return std::make_shared<JustValueView>(
        PrinterUtils::initializePointer(paramType.baseType(), res, paramType.getDimension()));
}

bool Tests::MethodDescription::operator==(const Tests::MethodDescription &other) const {
    if (this->name != other.name) {
        return false;
    }
    if (this->params.size() != other.params.size()) {
        return false;
    }
    for (int i = 0; i < this->params.size(); i++) {
        if (this->params[i].type.typeName() != other.params[i].type.typeName()) {
            return false;
        }
    }
    return true;
}

std::size_t
Tests::MethodDescriptionHash::operator()(const Tests::MethodDescription &methodDescription) const {
    std::string signatureHash = methodDescription.name;
    for (const auto &parameter : methodDescription.params) {
        signatureHash += parameter.type.typeName();
    }
    return std::hash<std::string>()(signatureHash);
}

TestMethod::TestMethod(std::string methodName, fs::path bitcodeFile, fs::path sourceFilename, bool is32)
        : methodName(std::move(methodName)), bitcodeFilePath(std::move(bitcodeFile)),
          sourceFilePath(std::move(sourceFilename)), is32bits(is32) {}

bool TestMethod::operator==(const TestMethod &rhs) const {
    return std::tie(    methodName,     bitcodeFilePath,     sourceFilePath,     is32bits)
        == std::tie(rhs.methodName, rhs.bitcodeFilePath, rhs.sourceFilePath, rhs.is32bits);
}
bool TestMethod::operator!=(const TestMethod &rhs) const {
    return !(rhs == *this);
}

UTBotKTestObject::UTBotKTestObject(std::string name,
                                   std::vector<char> bytes,
                                   std::vector<Pointer> pointers,
                                   size_t address,
                                   bool is_lazy)
    : name(std::move(name)), bytes(std::move(bytes)), pointers(std::move(pointers)),
      address(address), is_lazy(is_lazy) {
}

UTBotKTestObject::UTBotKTestObject(const KTestObject &kTestObject)
    : UTBotKTestObject(kTestObject.name,
                       { kTestObject.bytes, kTestObject.bytes + kTestObject.numBytes },
                       { kTestObject.pointers, kTestObject.pointers + kTestObject.numPointers },
                       kTestObject.address,
                       isUnnamed(kTestObject.name) == 0) {
}

bool isUnnamed(char *name) {
    return strcmp(name, LAZYNAME.c_str());
}

bool Tests::MethodTestCase::isError() const {
    return suiteName == ERROR_SUITE_NAME;
}

bool Tests::TypeAndVarName::operator<(const Tests::TypeAndVarName &other) const {
    return varName < other.varName || (varName == other.varName && type.mTypeName() < other.type.mTypeName());
}
} // tests
