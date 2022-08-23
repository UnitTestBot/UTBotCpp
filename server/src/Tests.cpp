#include "Tests.h"

#include "NameDecorator.h"
#include "exceptions/UnImplementedException.h"
#include "printers/TestsPrinter.h"
#include "utils/KleeUtils.h"

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
                   { Tests::ERROR_SUITE_NAME,   std::string() }} { }

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
    return value;
}

static const std::unordered_map<std::string, std::string> FPSpecialValuesMappings = {
        {"nan", "NAN"},
        {"-nan", "-NAN"},
        {"inf", "INFINITY"},
        {"-inf", "-INFINITY"}
};

namespace tests {
/**
 * The function checks for precense of argument in values as it is
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
                                                           types::EnumInfo &enumInfo,
                                                           size_t offsetInBits,
                                                           size_t lenInBits) {
    std::string value = readBytesAsValue<int>(byteArray, offsetInBits, lenInBits);
    if (CollectionUtils::containsKey(enumInfo.valuesToEntries, value)) {
        auto name = enumInfo.getEntryName(value, utbot::Language::CXX);
        value = NameDecorator::decorate(name);
    } else {
        LOG_S(WARNING) << "Enum value for '" << enumInfo.name << "' is out of range: " << value;
        value = StringUtils::stringFormat("(enum %s)(%d)", enumInfo.name, value);
    }
    return std::make_shared<EnumValueView>(value);
}

std::shared_ptr<UnionValueView> KTestObjectParser::unionView(const std::vector<char> &byteArray,
                                                             types::UnionInfo &unionInfo,
                                                             size_t offsetInBits,
                                                             PointerUsage usage) {
    auto bytesType = types::Type::createSimpleTypeFromName("utbot_byte");
    auto view = arrayView(byteArray, bytesType, unionInfo.size, offsetInBits, usage);
    auto subViews = collectUnionSubViews(byteArray, unionInfo, offsetInBits, usage);
    return std::make_shared<UnionValueView>(unionInfo.name, std::move(view), std::move(subViews));
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
                                                                  const types::Type &type,
                                                                  size_t arraySizeInBits,
                                                                  size_t offsetInBits,
                                                                  PointerUsage usage) {
    std::string message;
    types::EnumInfo enumInfo;
    types::UnionInfo unionInfo;
    types::StructInfo structInfo;
    std::vector<std::shared_ptr<AbstractValueView>> views;

    const types::Type baseType = type.baseTypeObj();

    size_t elementLenInBits;
    if (types::TypesHandler::isVoid(baseType)) {
        elementLenInBits = typesHandler.typeSize(Type::minimalScalarType());
    } else {
        elementLenInBits = typesHandler.typeSize(baseType);
    }
    for (size_t curPos = 0; curPos < arraySizeInBits; curPos += elementLenInBits) {
        switch (typesHandler.getTypeKind(baseType)) {
        case TypeKind::STRUCT:
            structInfo = typesHandler.getStructInfo(baseType);
            views.push_back(structView(byteArray, structInfo, curPos + offsetInBits, usage));
            break;
        case TypeKind::PRIMITIVE:
            views.push_back(primitiveView(byteArray, baseType, curPos + offsetInBits, elementLenInBits));
            break;
        case TypeKind::ENUM:
            enumInfo = typesHandler.getEnumInfo(type);
            views.push_back(enumView(byteArray, enumInfo, curPos + offsetInBits, elementLenInBits));
            break;
        case TypeKind::UNION:
            unionInfo = typesHandler.getUnionInfo(type);
            views.push_back(unionView(byteArray, unionInfo, curPos + offsetInBits, usage));
            break;
        case TypeKind::OBJECT_POINTER:
        case TypeKind::ARRAY:
            message = "Invariant ERROR: base type is pointer/array: " + type.typeName();
            LOG_S(ERROR) << message;
            // No break here
        case TypeKind::UNKNOWN:
            throw UnImplementedException(
                std::string("Arrays don't support element type: " + type.typeName())
            );
        default:
            message = "Missing case for this TypeKind in switch";
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
    }

    std::vector<size_t> sizes = type.arraysSizes(usage);
    for (size_t i = sizes.size() - 1; i > 0; i--) {
        size_t size = sizes[i];
        std::vector<std::shared_ptr<AbstractValueView>> newViews;
        for (size_t j = 0; j < views.size(); j += size) {
            auto chunkBeginIt = views.begin();
            std::advance(chunkBeginIt, j);
            auto chunkEndIt = views.begin();
            std::advance(chunkEndIt, j + size);
            std::vector<std::shared_ptr<AbstractValueView>> curViews = std::vector(chunkBeginIt, chunkEndIt);
            newViews.push_back(std::make_shared<ArrayValueView>(curViews));
        }
        views = newViews;
    }

    return std::make_shared<ArrayValueView>(views);
}

std::shared_ptr<FunctionPointerView> KTestObjectParser::functionPointerView(const std::optional<std::string> &scopeName,
                                                                            const std::string &methodName, const std::string &paramName) {
    std::string value =
        PrinterUtils::getFunctionPointerStubName(scopeName, methodName, paramName).substr(1);
    return std::make_shared<FunctionPointerView>(value);
}

std::shared_ptr<FunctionPointerView> KTestObjectParser::functionPointerView(const std::string &structName,
                                                                            const std::string &fieldName) {
    std::string value = PrinterUtils::getFunctionPointerAsStructFieldStubName(structName, fieldName, false).substr(1);
    return std::make_shared<FunctionPointerView>(value);
}

std::shared_ptr<ArrayValueView> KTestObjectParser::arrayView(const std::vector<char> &byteArray,
                                                             const types::Type &type,
                                                             size_t arraySizeInBits,
                                                             size_t offsetInBits,
                                                             PointerUsage usage) {
    types::StructInfo structInfo;
    types::EnumInfo enumInfo;
    types::UnionInfo unionInfo;
    std::vector<std::shared_ptr<AbstractValueView>> subViews;
    size_t elementLenInBits;
    if (types::TypesHandler::isVoid(type)) {
        elementLenInBits = typesHandler.typeSize(Type::minimalScalarType());
    } else {
        elementLenInBits = typesHandler.typeSize(type);
    }
    for (size_t curPos = 0; curPos < arraySizeInBits; curPos += elementLenInBits) {
        switch (typesHandler.getTypeKind(type)) { // tdm_todo copypaste
            case TypeKind::STRUCT:
                structInfo = typesHandler.getStructInfo(type);
                subViews.push_back(structView(byteArray, structInfo, curPos + offsetInBits, usage));
                break;
            case TypeKind::PRIMITIVE:
                subViews.push_back(primitiveView(byteArray, type.baseTypeObj(), curPos + offsetInBits, elementLenInBits));
                break;
            case TypeKind::ENUM:
                enumInfo = typesHandler.getEnumInfo(type);
                subViews.push_back(enumView(byteArray, enumInfo, curPos + offsetInBits, elementLenInBits));
                break;
            case TypeKind::UNION:
                unionInfo = typesHandler.getUnionInfo(type);
                subViews.push_back(unionView(byteArray, unionInfo, curPos + offsetInBits, usage));
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
                                                               types::StructInfo &curStruct,
                                                               size_t offsetInBits,
                                                               types::PointerUsage usage) {
    std::vector<InitReference> tmpInitReferences;
    return structView(byteArray, curStruct, offsetInBits, usage, {}, "", {}, tmpInitReferences);
}

std::shared_ptr<StructValueView> KTestObjectParser::structView(const std::vector<char> &byteArray,
                                                               StructInfo &curStruct,
                                                               size_t offsetInBits,
                                                               PointerUsage usage,
                                                               const std::optional<const Tests::MethodDescription> &testingMethod,
                                                               const std::string &name,
                                                               const MapAddressName &fromAddressToName,
                                                               std::vector<InitReference> &initReferences) {
    std::vector<std::shared_ptr<AbstractValueView>> subViews;
    std::vector<std::string> fields;
    size_t structOffset = offsetInBits;

    for (const auto &field: curStruct.fields) {
        fields.push_back(field.name);
        size_t fieldLen = typesHandler.typeSize(field.type);
        size_t fieldOffset = structOffset + field.offset;
        types::EnumInfo innerEnum;
        types::UnionInfo innerUnion;
        types::StructInfo innerStruct;
        std::string res;

        switch (typesHandler.getTypeKind(field.type)) {
            case TypeKind::PRIMITIVE:
                // tdm_todo unnamed bitfield
                subViews.push_back(primitiveView(byteArray, field.type.baseTypeObj(), fieldOffset,
                                                 std::min(field.size, fieldLen)));
                break;
            case TypeKind::STRUCT:
                innerStruct = typesHandler.getStructInfo(field.type);
                subViews.push_back(structView(byteArray, innerStruct, fieldOffset, usage, testingMethod,
                                              PrinterUtils::getFieldAccess(name, field.name), fromAddressToName, initReferences));
                break;
            case TypeKind::ENUM:
                innerEnum = typesHandler.getEnumInfo(field.type);
                subViews.push_back(enumView(byteArray, innerEnum, fieldOffset, fieldLen));
                break;
            case TypeKind::UNION:
                innerUnion = typesHandler.getUnionInfo(field.type);
                subViews.push_back(unionView(byteArray, innerUnion, fieldOffset, usage));
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
                        subViews.push_back(multiArrayView(byteArray, field.type, size,
                                                          fieldOffset, usage));
                    } else {
                        std::vector<std::shared_ptr<AbstractValueView>> nullViews(
                            size, std::make_shared<JustValueView>(PrinterUtils::C_NULL));
                        subViews.push_back(std::make_shared<ArrayValueView>(nullViews));
                    }
                } else {
                    auto view = arrayView(byteArray, field.type.baseTypeObj(), fieldLen, fieldOffset, usage);
                    subViews.push_back(view);
                }
            }
                break;
            case TypeKind::OBJECT_POINTER:
                res = readBytesAsValueForType(byteArray, PointerWidthType, fieldOffset,
                                              PointerWidthSizeInBits);
                subViews.push_back(getLazyPointerView(fromAddressToName, initReferences,
                                                      PrinterUtils::getFieldAccess(name, field.name), res, field.type));
                break;
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
    }

    std::optional<std::string> entryValue;
    if (curStruct.hasAnonymousStructOrUnion) {
        auto bytesType = types::Type::createSimpleTypeFromName("utbot_byte");
        const std::shared_ptr<AbstractValueView> rawDataView = arrayView(byteArray, bytesType, curStruct.size, offsetInBits, usage);
        entryValue = PrinterUtils::convertBytesToUnion(curStruct.name, rawDataView->getEntryValue(nullptr));
    }
    return std::make_shared<StructValueView>(curStruct.isCLike, fields, subViews, entryValue);
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

    bool stobool(const std::string& s) {
        if (s == "false") return false;
        if (s == "true") return true;
        ABORT_F("Wrong bool value: %s", s.c_str());
    }

    bool predicateMatch(const std::string &value, const LineInfo::PredicateInfo &info) {
        switch(info.type) {
            case testsgen::CHAR:
                return compareSimpleValues(info.predicate, value, "\'" + info.returnValue + "\'");
            case testsgen::STRING:
                return compareSimpleValues(info.predicate, value, "\"" + info.returnValue + "\"");
            case testsgen::INT8_T:
            case testsgen::INT16_T:
            case testsgen::INT32_T:
            case testsgen::INT64_T:
                return compareSimpleValues(info.predicate, std::stoll(value), std::stoll(info.returnValue));
            case testsgen::UINT8_T:
            case testsgen::UINT16_T:
            case testsgen::UINT32_T:
            case testsgen::UINT64_T:
                return compareSimpleValues(info.predicate, std::stoull(value), std::stoull(info.returnValue));
            case testsgen::BOOL:
                return compareSimpleValues(info.predicate, stobool(value), stobool(info.returnValue));
            case testsgen::FLOAT:
                return compareSimpleValues(info.predicate, std::stof(value), std::stof(info.returnValue));
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

size_t KTestObjectParser::findFieldIndex(const StructInfo &structInfo, size_t offsetInBits) {
    size_t indField = std::upper_bound(structInfo.fields.begin(), structInfo.fields.end(), offsetInBits, [] (int offset, const Field &field) {
        return offset < field.offset;
    }) - structInfo.fields.begin();
    indField--;
    if (indField < 0) {
        std::string message = "Wrong offset";
        LOG_S(ERROR) << message;
        throw IncorrectIndexException(message);
    }
    return indField;
}

void KTestObjectParser::addToOrder(const std::vector<UTBotKTestObject> &objects,
                                   const std::string &paramName,
                                   const types::Type &paramType,
                                   Tests::TestCaseParamValue &paramValue,
                                   std::vector<bool> &visited,
                                   std::queue<JsonIndAndParam> &order) {
    auto it = std::find_if(objects.begin(), objects.end(),
                     [paramName](const UTBotKTestObject &obj) { return obj.name == paramName; });
    if (it != objects.end()) {
        size_t jsonInd = it - objects.begin();
        visited[jsonInd] = true;
        Tests::MethodParam param = { paramType.isObjectPointer() ? paramType.baseTypeObj()
                                                                 : paramType,
                                     paramName, std::nullopt };
        order.emplace(jsonInd, param, paramValue);
        return;
    }
    std::string message = "Don't find object " + paramName + " in objects array";
    LOG_S(WARNING) << message;
}

void KTestObjectParser::assignTypeUnnamedVar(Tests::MethodTestCase &testCase,
                                             const Tests::MethodDescription &methodDescription) {
    std::queue<JsonIndAndParam> order;
    std::vector<bool> visited(testCase.objects.size(), false);
    for (size_t paramInd = 0; paramInd < testCase.paramValues.size(); paramInd++) {
        addToOrder(testCase.objects, methodDescription.params[paramInd].name,
                   methodDescription.params[paramInd].type,
                   testCase.paramValues[paramInd], visited, order);
    }
    addToOrder(testCase.objects, KleeUtils::RESULT_VARIABLE_NAME,
               methodDescription.returnType,
               testCase.returnValue, visited, order);

    while (!order.empty()) {
        auto curType = order.front();
        order.pop();
        types::Type paramType = curType.param.type;
        if (testCase.objects[curType.jsonInd].is_lazy) {
            if (types::TypesHandler::baseTypeIsVoid(paramType)) {
                throw UnImplementedException("Lazy variable has baseType=void");
            }
            auto lazyNameIt =
                testCase.lazyAddressToName.find(testCase.objects[curType.jsonInd].address);
            std::string name = lazyNameIt == testCase.lazyAddressToName.end()
                                   ? testCase.objects[curType.jsonInd].name
                                   : lazyNameIt->second;

            std::vector<char> byteValue = testCase.objects[curType.jsonInd].bytes;
            Tests::TypeAndVarName typeAndVarName{ paramType, name };
            std::shared_ptr<AbstractValueView> testParamView = testParameterView(
                { name, byteValue }, typeAndVarName,
                                                                                 PointerUsage::LAZY, testCase.lazyAddressToName,
                                                                                 testCase.lazyReferences, methodDescription);
            LOG_S(MAX) << "Fetch lazy object: " << name << " = " << testParamView->getEntryValue(nullptr);
            curType.paramValue.lazyParams.emplace_back(paramType, name, std::nullopt);
            curType.paramValue.lazyValues.emplace_back(name, std::nullopt, testParamView);
        }

        for (auto const &[offset, indObj] : testCase.objects[curType.jsonInd].offsetsInBytes) {
            if (!visited[indObj]) {
                types::Type fieldType =
                    traverseLazyInStruct(visited, paramType, SizeUtils::bytesToBits(offset), testCase, methodDescription);
                Tests::MethodParam param = { fieldType, "", std::nullopt };
                order.emplace(indObj, param, curType.paramValue);
                visited[indObj] = true;
            }
        }
    }
}

types::Type KTestObjectParser::traverseLazyInStruct(std::vector<bool> &visited,
                                                    const types::Type &curVarType,
                                                    size_t offsetInBits,
                                                    const Tests::MethodTestCase &testCase,
                                                    const Tests::MethodDescription &methodDescription) {
    std::string message;
    switch (typesHandler.getTypeKind(curVarType)) {
        case TypeKind::STRUCT: {
            types::StructInfo structInfo = typesHandler.getStructInfo(curVarType);
            size_t indField = findFieldIndex(structInfo, offsetInBits);
            const types::Field &next = structInfo.fields[indField];
            return traverseLazyInStruct(visited, next.type, offsetInBits - next.offset, testCase,
                                        methodDescription);
        }
        case TypeKind::OBJECT_POINTER: {
            LOG_IF_S(ERROR, offsetInBits != 0) << "Offset not zero" << offsetInBits;
            return curVarType.baseTypeObj(1);
        }
        case TypeKind::PRIMITIVE: {
            return curVarType;
        }
        case TypeKind::UNION:
        case TypeKind::ENUM:
        case TypeKind::FUNCTION_POINTER:
        case TypeKind::ARRAY:
        case TypeKind::UNKNOWN:
        default:
            message = "Unsupported type in lazy initialization BFS: " + curVarType.typeName();
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
}

void KTestObjectParser::assignTypeStubVar(Tests::MethodTestCase &testCase,
                                          const Tests::MethodDescription &methodDescription) {
    for (auto const &obj : testCase.objects) {
        if (StringUtils::endsWith(obj.name, PrinterUtils::KLEE_SYMBOLIC_SUFFIX)) {
            std::string stubFuncName = obj.name.substr(0, obj.name.length() - PrinterUtils::KLEE_SYMBOLIC_SUFFIX.length());
            if (!CollectionUtils::contains(methodDescription.functionPointers, stubFuncName)) {
                std::string message = "Can't find function pointer with name " + stubFuncName;
                LOG_S(WARNING) << message;
                continue;
            }
            types::Type stubType = types::Type::createArray(methodDescription.functionPointers.at(stubFuncName)->returnType);
            std::shared_ptr<AbstractValueView> stubView = testParameterView({obj.name, obj.bytes}, {stubType, obj.name},
                                                                            PointerUsage::PARAMETER, testCase.lazyAddressToName,
                                                                            testCase.lazyReferences, methodDescription);
            testCase.stubParamValues.emplace_back(obj.name, 0, stubView);
            testCase.stubParamTypes.emplace_back(stubType, obj.name, std::nullopt);
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
            std::swap(testCase.objects, testCaseDescription.objects);
            std::swap(testCase.lazyAddressToName, testCaseDescription.lazyAddressToName);
            std::swap(testCase.lazyReferences, testCaseDescription.lazyReferences);

            testCase.errorDescriptors = case_.errorDescriptors;

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

            assignTypeUnnamedVar(testCase, methodDescription);
            assignTypeStubVar(testCase, methodDescription);

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

Tests::TestCaseDescription
KTestObjectParser::parseTestCaseParams(const UTBotKTest &ktest,
                                       const Tests::MethodDescription &methodDescription,
                                       const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
                                       const std::stringstream &traceStream) {
    std::vector<RawKleeParam> rawKleeParams;
    for (auto const &param : ktest.objects) {
        rawKleeParams.emplace_back(param.name, param.bytes);
    }

    Tests::TestCaseDescription testCaseDescription;
    testCaseDescription.objects = ktest.objects;

    for (const auto &obj : testCaseDescription.objects) {
        if (obj.name != LAZYNAME) {
            testCaseDescription.lazyAddressToName.insert({obj.address, obj.name});
        }
    }

    int cnt = 0;
    std::vector<bool> visited(testCaseDescription.objects.size(), false);
    for (const auto &obj : testCaseDescription.objects) {
        for (const auto &off : obj.offsetsInBytes) {
            if (visited[off.index]) continue;
            visited[off.index] = true;
            testCaseDescription.objects[off.index].name = PrinterUtils::generateNewVar(++cnt);
            size_t address = std::stoull(
                    readBytesAsValueForType(obj.bytes, PointerWidthType, SizeUtils::bytesToBits(off.offset),
                                            PointerWidthSizeInBits));
            testCaseDescription.lazyAddressToName.emplace(address, testCaseDescription.objects[off.index].name);
        }
    }

    const RawKleeParam emptyKleeParam = {"", {}};

    if (methodDescription.isClassMethod()) {
        auto methodParam = methodDescription.classObj.value();
        std::shared_ptr<AbstractValueView> testParamView;
        getTestParamView(methodDescription, rawKleeParams, emptyKleeParam, testCaseDescription, methodParam,
                         testParamView);
        testCaseDescription.classPreValues = { methodParam.name, methodParam.alignment, testParamView };
        processClassPostValue(testCaseDescription, methodParam, rawKleeParams);
    }

    for (auto &methodParam : methodDescription.params) {
        std::shared_ptr<AbstractValueView> testParamView;
        getTestParamView(methodDescription, rawKleeParams, emptyKleeParam, testCaseDescription, methodParam,
                         testParamView);
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
    }

    processStubParamValue(testCaseDescription, methodNameToReturnTypeMap, rawKleeParams);
    if (!types::TypesHandler::skipTypeInReturn(methodDescription.returnType)) {
        const auto kleeResParam = getKleeParamOrThrow(rawKleeParams, KleeUtils::RESULT_VARIABLE_NAME);
        auto paramType = methodDescription.returnType.maybeReturnArray() ? methodDescription.returnType :
                                                                         methodDescription.returnType.baseTypeObj();
        const Tests::TypeAndVarName returnParam = { paramType, KleeUtils::RESULT_VARIABLE_NAME };
        const auto testReturnView = testParameterView(kleeResParam, returnParam, PointerUsage::RETURN,
                                                      testCaseDescription.lazyAddressToName, testCaseDescription.lazyReferences,
                                                      methodDescription);
        testCaseDescription.returnValue = {
            KleeUtils::RESULT_VARIABLE_NAME, types::TypesHandler::isObjectPointerType(methodDescription.returnType), testReturnView
        };
    } else {
        testCaseDescription.returnValue = { KleeUtils::RESULT_VARIABLE_NAME, false,
                                            std::make_shared<VoidValueView>() };
    }

    const auto kleePathFlagIterator = getKleeParam(rawKleeParams, KLEE_PATH_FLAG);
    const auto kleePathFlagSymbolicIterator = getKleeParam(rawKleeParams, KLEE_PATH_FLAG_SYMBOLIC);
    if (kleePathFlagSymbolicIterator != rawKleeParams.end()) {
        const Tests::TypeAndVarName kleePathParam = {types::Type::intType(), KLEE_PATH_FLAG_SYMBOLIC};
        const auto kleePathFlagSymbolicView = testParameterView(*kleePathFlagSymbolicIterator, kleePathParam, types::PointerUsage::PARAMETER,
                                                                testCaseDescription.lazyAddressToName, testCaseDescription.lazyReferences);
        testCaseDescription.kleePathFlagSymbolicValue = {KLEE_PATH_FLAG_SYMBOLIC, false, kleePathFlagSymbolicView};
    }
    const auto functionReturnNotNullIterator = getKleeParam(rawKleeParams, KleeUtils::NOT_NULL_VARIABLE_NAME);
    if (functionReturnNotNullIterator != rawKleeParams.end()) {
        const Tests::TypeAndVarName functionReturnNotNull = {types::Type::intType(), KleeUtils::NOT_NULL_VARIABLE_NAME};
        const auto functionReturnNotNullView = testParameterView(*functionReturnNotNullIterator, functionReturnNotNull, types::PointerUsage::PARAMETER,
                                                                 testCaseDescription.lazyAddressToName, testCaseDescription.lazyReferences);
        testCaseDescription.functionReturnNotNullValue = {KleeUtils::NOT_NULL_VARIABLE_NAME, false, functionReturnNotNullView};
    }
    return testCaseDescription;
}

void KTestObjectParser::getTestParamView(const Tests::MethodDescription &methodDescription,
                                         const std::vector<RawKleeParam> &rawKleeParams,
                                         const KTestObjectParser::RawKleeParam &emptyKleeParam,
                                         Tests::TestCaseDescription &testCaseDescription,
                                         const Tests::MethodParam& methodParam,
                                         std::shared_ptr<AbstractValueView> &testParamView) {
    auto paramType = methodParam.type.maybeJustPointer() ? methodParam.type.baseTypeObj() : methodParam.type;
    if (CollectionUtils::containsKey(methodDescription.functionPointers, methodParam.name)) {
        testParamView = testParameterView(
                emptyKleeParam, { paramType, methodParam.name }, PointerUsage::PARAMETER, testCaseDescription.lazyAddressToName,
                testCaseDescription.lazyReferences, methodDescription);
    } else {
        const auto kleeParam = getKleeParamOrThrow(rawKleeParams, methodParam.name);
        testParamView = testParameterView(kleeParam, { paramType, methodParam.name }, PointerUsage::PARAMETER,
                                          testCaseDescription.lazyAddressToName, testCaseDescription.lazyReferences,
                                          methodDescription);
    }
}

void KTestObjectParser::processGlobalParamPreValue(Tests::TestCaseDescription &testCaseDescription,
                                                   const Tests::MethodParam &globalParam,
                                                   std::vector<RawKleeParam> &rawKleeParams) {
    std::string kleeParamName = globalParam.name;
    auto kleeParam = getKleeParamOrThrow(rawKleeParams, kleeParamName);
    auto testParamView = testParameterView(kleeParam, { globalParam.type, globalParam.name }, types::PointerUsage::PARAMETER,
                                           testCaseDescription.lazyAddressToName, testCaseDescription.lazyReferences);
    testCaseDescription.globalPreValues.emplace_back( globalParam.name, globalParam.alignment, testParamView );
}

void KTestObjectParser::processSymbolicStdin(Tests::TestCaseDescription &testCaseDescription, std::vector<RawKleeParam> &rawKleeParams) {
    auto &&read = getKleeParamOrThrow(rawKleeParams, "stdin-read");
    std::string &&view = testParameterView(read, {types::Type::longlongType(), "stdin-read"}, types::PointerUsage::PARAMETER,
                                      testCaseDescription.lazyAddressToName, testCaseDescription.lazyReferences)->getEntryValue(nullptr);
    if (view == "0LL") {
        return;
    } else {
        long long usedStdinBytesCount = std::stoll(view);
        if (usedStdinBytesCount > types::Type::symStdinSize) {
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

void KTestObjectParser::processGlobalParamPostValue(Tests::TestCaseDescription &testCaseDescription,
                                                    const Tests::MethodParam &globalParam,
                                                    std::vector<RawKleeParam> &rawKleeParams) {
    auto symbolicVariable = KleeUtils::postSymbolicVariable(globalParam.name);
    auto kleeParam = getKleeParamOrThrow(rawKleeParams, symbolicVariable);
    auto type = typesHandler.getReturnTypeToCheck(globalParam.type);
    Tests::TypeAndVarName typeAndVarName{ type, globalParam.name };
    auto testParamView = testParameterView(kleeParam, typeAndVarName, types::PointerUsage::PARAMETER,
                                           testCaseDescription.lazyAddressToName, testCaseDescription.lazyReferences);
    testCaseDescription.globalPostValues.emplace_back( globalParam.name, globalParam.alignment, testParamView );
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
    auto testParamView = testParameterView(kleeParam, typeAndVarName, usage, testCaseDescription.lazyAddressToName,
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
    auto testParamView = testParameterView(kleeParam, typeAndVarName, usage, testCaseDescription.lazyAddressToName,
                                           testCaseDescription.lazyReferences);
    testCaseDescription.paramPostValues.emplace_back( param.name, param.alignment, testParamView );
}

void KTestObjectParser::processStubParamValue(Tests::TestCaseDescription &testCaseDescription,
                                              const std::unordered_map<std::string, types::Type>& methodNameToReturnTypeMap,
                                              std::vector<RawKleeParam> &rawKleeParams) {
    for (const auto &kleeParam: rawKleeParams) {
        if (StringUtils::endsWith(kleeParam.paramName, PrinterUtils::KLEE_SYMBOLIC_SUFFIX)) {
            std::string methodName = kleeParam.paramName.substr(0, kleeParam.paramName.size() - PrinterUtils::KLEE_SYMBOLIC_SUFFIX.size());
            if (!CollectionUtils::containsKey(methodNameToReturnTypeMap, methodName)) {
                LOG_S(WARNING) << "Method name \"" << methodName << "\" was not fetched, skipping";
                continue;
            }
            auto type = typesHandler.getReturnTypeToCheck(methodNameToReturnTypeMap.at(methodName));
            Tests::TypeAndVarName typeAndVarName{ type, kleeParam.paramName };
            auto testParamView = testParameterView(kleeParam, typeAndVarName, types::PointerUsage::PARAMETER,
                                                   testCaseDescription.lazyAddressToName, testCaseDescription.lazyReferences);
            testCaseDescription.stubValues.emplace_back( kleeParam.paramName, 0, testParamView );
            testCaseDescription.stubValuesTypes.emplace_back(type, kleeParam.paramName, 0);
        }
    }
}

std::shared_ptr<AbstractValueView> KTestObjectParser::testParameterView(
        const KTestObjectParser::RawKleeParam &kleeParam,
        const Tests::TypeAndVarName &param,
        PointerUsage usage,
        const MapAddressName &fromAddressToName,
        std::vector<InitReference> &initReferences,
        const std::optional<const Tests::MethodDescription> &testingMethod) {
    EnumInfo enumInfo; // tdm_todo refactor so don't create useless variable. create it in switch
    StructInfo structInfo;
    UnionInfo unionInfo;
    std::string message, name;
    const auto &rawData = kleeParam.rawData;
    const auto &paramType = param.type;
    switch (typesHandler.getTypeKind(paramType)) {
        case TypeKind::PRIMITIVE:
            return primitiveView(rawData, paramType.baseTypeObj(), 0, SizeUtils::bytesToBits(rawData.size()));
        case TypeKind::STRUCT:
            structInfo = typesHandler.getStructInfo(paramType);
            name = param.varName;
            return structView(rawData, structInfo, 0, usage, testingMethod, name, fromAddressToName,
                              initReferences);
        case TypeKind::OBJECT_POINTER:
            if (usage == types::PointerUsage::LAZY) {
                std::string res = readBytesAsValueForType(rawData, PointerWidthType, 0, PointerWidthSizeInBits);
                return getLazyPointerView(fromAddressToName, initReferences, param.varName, res, paramType);
            } else if (types::TypesHandler::isCStringType(paramType)) {
                return stringLiteralView(rawData);
            } else if (paramType.kinds().size() > 2) {
                return multiArrayView(rawData, paramType, SizeUtils::bytesToBits(rawData.size()), 0, usage);
            } else {
                return arrayView(rawData, paramType.baseTypeObj(), SizeUtils::bytesToBits(rawData.size()), 0, usage);
            }
        case TypeKind::FUNCTION_POINTER:
            if (!testingMethod.has_value()) {
                return functionPointerView(std::nullopt, "", param.varName);
            }
            return functionPointerView(testingMethod->getClassTypeName(), testingMethod->name, param.varName);
        case TypeKind::ENUM:
            enumInfo = typesHandler.getEnumInfo(paramType);
            return enumView(rawData, enumInfo, 0, SizeUtils::bytesToBits(rawData.size()));
        case TypeKind::UNION:
            unionInfo = typesHandler.getUnionInfo(paramType);
            return unionView(rawData, unionInfo, 0, usage);
        case TypeKind::ARRAY:
            if (paramType.kinds().size() > 2) {
                return multiArrayView(rawData, paramType, SizeUtils::bytesToBits(rawData.size()), 0, usage);
            } else {
                return arrayView(rawData, paramType.baseTypeObj(), SizeUtils::bytesToBits(rawData.size()), 0, usage);
            }
        case TypeKind::UNKNOWN:
            throw UnImplementedException("No such type");
        default:
            message = "Missing case for this TypeKind in switch";
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
    }
}

std::shared_ptr<AbstractValueView>
KTestObjectParser::getLazyPointerView(const MapAddressName &fromAddressToName,
                                      std::vector<InitReference> &initReferences,
                                      const std::string &name,
                                      std::string res,
                                      const Type &paramType) const {
    if (fromAddressToName.find(std::stoull(res)) != fromAddressToName.end()) {
        initReferences.emplace_back(name, fromAddressToName.at(std::stoull(res)),
            PrinterUtils::initializePointerToVar(paramType.baseType(),
                                                 fromAddressToName.at(std::stoull(res)), paramType.getDimension()));
        res = PrinterUtils::C_NULL;
    }
    return std::make_shared<JustValueView>(PrinterUtils::initializePointer(
        paramType.baseType(), res, paramType.getDimension()));
}

std::vector<std::shared_ptr<AbstractValueView>>
KTestObjectParser::collectUnionSubViews(const std::vector<char> &byteArray,
                                        const types::UnionInfo &info,
                                        size_t offsetInBits,
                                        types::PointerUsage usage) {
    std::vector<std::shared_ptr<AbstractValueView>> subViews;
    for (const auto &field : info.fields) {
        size_t fieldLen = typesHandler.typeSize(field.type);
        types::EnumInfo innerEnum;
        types::UnionInfo innerUnion;
        types::StructInfo innerStruct;
        switch (typesHandler.getTypeKind(field.type)) {
        case TypeKind::PRIMITIVE:
            subViews.push_back(primitiveView(byteArray, field.type.baseTypeObj(), offsetInBits,
                                             std::min(field.size, fieldLen)));
            break;
        case TypeKind::STRUCT:
            innerStruct = typesHandler.getStructInfo(field.type);
            subViews.push_back(structView(byteArray, innerStruct, offsetInBits, usage));
            break;
        case TypeKind::ENUM:
            innerEnum = typesHandler.getEnumInfo(field.type);
            subViews.push_back(enumView(byteArray, innerEnum, offsetInBits, fieldLen));
            break;
        case TypeKind::UNION:
            innerUnion = typesHandler.getUnionInfo(field.type);
            subViews.push_back(unionView(byteArray, innerUnion, offsetInBits, usage));
            break;
        case TypeKind::ARRAY:
            subViews.push_back(arrayView(byteArray, field.type.baseTypeObj(), fieldLen, offsetInBits, usage));
            break;
        case TypeKind::OBJECT_POINTER:
            subViews.push_back(std::make_shared<JustValueView>(PrinterUtils::C_NULL));
            break;
        case TypeKind::UNKNOWN:
            throw UnImplementedException(
                std::string("Structs don't support fields of type: " + field.type.typeName()));

        default:
            std::string message = "Missing case for this TypeKind in switch";
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
    }
    return subViews;
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

UnionValueView::UnionValueView(
        const std::string &typeName,
        const std::shared_ptr<AbstractValueView> &rawDataView,
        std::vector<std::shared_ptr<AbstractValueView>, std::allocator<std::shared_ptr<AbstractValueView>>> subViews)
        : AbstractValueView(std::move(subViews)),
          entryValue(PrinterUtils::convertBytesToUnion(typeName, rawDataView->getEntryValue(nullptr))) {}

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

UTBotKTestObject::UTBotKTestObject(std::string name, std::vector<char> bytes, std::vector<Offset> offsets,
                                   uint64_t address, bool is_lazy) : name(std::move(name)), bytes(std::move(bytes)),
                                                                     offsetsInBytes(std::move(offsets)), address(address), is_lazy(is_lazy) {
}

UTBotKTestObject::UTBotKTestObject(const ConcretizedObject &kTestObject)
    : UTBotKTestObject(kTestObject.name,
                       { kTestObject.values, kTestObject.values + kTestObject.size },
                       { kTestObject.offsets, kTestObject.offsets + kTestObject.n_offsets },
                       kTestObject.address,
                       isUnnamed(kTestObject.name) == 0) {
}

bool isUnnamed(char *name) {
    return strcmp(name, LAZYNAME.c_str());
}

bool Tests::MethodTestCase::isError() const {
    return suiteName == ERROR_SUITE_NAME;
}
}
