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
        : suiteTestCases{{Tests::DEFAULT_SUITE_NAME, std::vector<int>()},
                         {Tests::ERROR_SUITE_NAME,   std::vector<int>()}},
          codeText{{Tests::DEFAULT_SUITE_NAME, std::string()},
                   {Tests::ERROR_SUITE_NAME,   std::string()}},
          modifiers{} {
    stubsParamStorage = std::make_shared<StubsStorage>();
    stubsStorage = std::make_shared<StubsStorage>();
}

static const std::unordered_map<std::string, std::string> FPSpecialValuesMappings = {
        {"nan",  "NAN"},
        {"-nan", "-NAN"},
        {"inf",  "INFINITY"},
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
        if (FPSpecialValuesMappings.find(value) == FPSpecialValuesMappings.end()) {
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
    bool isFPSpecialValue(const std::string &value) {
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

    std::shared_ptr<PrimitiveValueView> KTestObjectParser::primitiveView(const UTBotKTestObject::RawData &rawData,
                                                                         const types::Type &type,
                                                                         size_t offsetInBits,
                                                                         size_t lenInBits) {
        Type readType = types::TypesHandler::isVoid(type) ? Type::minimalScalarType() : type;
        std::string value = readBytesAsValueForType(rawData.bytes, readType.baseType(), offsetInBits, lenInBits);
        value = makeDecimalConstant(value, type.baseType());
        value = processFPSpecialValue(value);
        if (types::TypesHandler::isBoolType(type)) {
            return std::make_shared<PrimitiveValueView>(primitiveBoolView(value));
        }
        return std::make_shared<PrimitiveValueView>(primitiveCharView(type.baseTypeObj(), value));
    }


    std::shared_ptr<EnumValueView> KTestObjectParser::enumView(const UTBotKTestObject::RawData &rawData,
                                                               const types::EnumInfo &enumInfo,
                                                               size_t offsetInBits,
                                                               size_t lenInBits) {
        std::string value = readBytesAsValue<int>(rawData.bytes, offsetInBits, lenInBits);
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

    std::shared_ptr<FunctionPointerView> KTestObjectParser::functionPointerView(
            const std::optional<std::string> &scopeName,
            const std::string &methodName, const std::string &paramName) {
        std::string value =
                StubsUtils::getFunctionPointerStubName(scopeName, methodName, paramName, false).substr(1);
        return std::make_shared<FunctionPointerView>(value);
    }

    std::shared_ptr<FunctionPointerView> KTestObjectParser::functionPointerView(const std::string &structName,
                                                                                const std::string &fieldName) {
        std::string value = StubsUtils::getFunctionPointerAsStructFieldStubName(structName, fieldName, false).substr(1);
        return std::make_shared<FunctionPointerView>(value);
    }

    std::shared_ptr<FixedArrayValueView>
    KTestObjectParser::fixedArrayView(const UTBotKTestObject::RawData &rawData,
                                      const types::Type &type,
                                      const std::string &name,
                                      size_t arraySizeInBits,
                                      size_t offsetInBits,
                                      const std::vector<UTBotKTestObject> &objects,
                                      std::vector<InitReference> &initReferences,
                                      const std::optional<const Tests::MethodDescription> &testingMethod) {
        std::vector<std::shared_ptr<AbstractValueView>> subViews;
        if (typesHandler.getTypeKind(type) != TypeKind::ARRAY) {
            //TODO change exception type
            throw UnImplementedException("Incorrect type in array");
        }
        auto subType = type.baseTypeObj(1);

        size_t elementLenInBits = typesHandler.typeSize(types::TypesHandler::isVoid(subType)
                                                        ? Type::minimalScalarType() : subType);

        size_t ind = 0;
        for (size_t curPos = offsetInBits; curPos < offsetInBits + arraySizeInBits; curPos += elementLenInBits) {
            std::string nameWithIndex = StringUtils::stringFormat("%s[%d]", name, ind);
            switch (typesHandler.getTypeKind(subType)) {
                case TypeKind::STRUCT_LIKE:
                    subViews.push_back(
                            structView(rawData, typesHandler.getStructInfo(subType), nameWithIndex, objects,
                                       initReferences,
                                       testingMethod, curPos, false));
                    break;
                case TypeKind::ENUM:
                    subViews.push_back(enumView(rawData, typesHandler.getEnumInfo(subType), curPos, elementLenInBits));
                    break;
                case TypeKind::PRIMITIVE:
                    subViews.push_back(primitiveView(rawData, subType.baseTypeObj(), curPos, elementLenInBits));
                    break;
                case TypeKind::OBJECT_POINTER: {
                    subViews.push_back(
                            getLazyPointerView(nameWithIndex, subType, true, objects, initReferences, rawData, curPos));
                    break;
                }
                case TypeKind::ARRAY: {
                    subViews.push_back(
                            fixedArrayView(rawData, subType, nameWithIndex, elementLenInBits, curPos, objects,
                                           initReferences,
                                           testingMethod));
                    break;
                }
                case TypeKind::UNKNOWN: {
                    std::string message = "Arrays don't support element type: " + type.typeName();
                    LOG_S(ERROR) << message;
                    throw UnImplementedException(message);
                }
                default: {
                    std::string message = "Missing case for this TypeKind in switch";
                    LOG_S(ERROR) << message;
                    throw NoSuchTypeException(message);
                }
            }
            ++ind;
        }
        return std::make_shared<FixedArrayValueView>(subViews);
    }

    std::shared_ptr<StructValueView> KTestObjectParser::structView(const UTBotKTestObject::RawData &rawData,
                                                                   const types::StructInfo &curStruct,
                                                                   const std::string &name,
                                                                   const std::vector<UTBotKTestObject> &objects,
                                                                   std::vector<InitReference> &initReferences,
                                                                   const std::optional<const Tests::MethodDescription> &testingMethod,
                                                                   size_t offsetInBits,
                                                                   const bool anonymous) {
        std::vector<std::shared_ptr<AbstractValueView>> subViews;

        const auto &byteArray = rawData.bytes;
        const auto &lazyPointersArray = rawData.pointers;

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

            std::string accessName = PrinterUtils::getFieldAccess(name, field);
            switch (typesHandler.getTypeKind(field.type)) {
                case TypeKind::STRUCT_LIKE: {
                    auto sv = structView(rawData, typesHandler.getStructInfo(field.type), accessName, objects,
                                         initReferences, testingMethod, fieldStartOffset, field.anonymous);
                    dirtyInitializedField |= sv->isDirtyInit();
                    isInitializedField = sv->isInitialized();
                    subViews.push_back(sv);
                }
                    break;
                case TypeKind::ENUM:
                    subViews.push_back(
                            enumView(rawData, typesHandler.getEnumInfo(field.type), fieldStartOffset, fieldLen));
                    break;
                case TypeKind::PRIMITIVE:
                    subViews.push_back(primitiveView(rawData, field.type.baseTypeObj(), fieldStartOffset,
                                                     std::min(field.size, fieldLen)));
                    break;
                case TypeKind::ARRAY: {
                    auto view = fixedArrayView(rawData, field.type, accessName, fieldLen, fieldStartOffset, objects,
                                               initReferences, testingMethod);
                    subViews.push_back(view);
                }
                    break;
                case TypeKind::OBJECT_POINTER: {
                    auto pointerIterator =
                            std::find_if(lazyPointersArray.begin(), lazyPointersArray.end(),
                                         [&fieldStartOffset](const Pointer &ptr) {
                                             return SizeUtils::bytesToBits(ptr.offset) == fieldStartOffset;
                                         }) != lazyPointersArray.end();
                    subViews.push_back(getLazyPointerView(accessName, field.type, pointerIterator,
                                                          objects, initReferences, rawData, fieldStartOffset));
                }
                    break;
                case TypeKind::FUNCTION_POINTER:
                    subViews.push_back(functionPointerView(curStruct.name, field.name));
                    break;
                case TypeKind::UNKNOWN: {
                    std::string message = "Structs don't support fields of type: " + field.type.typeName();
                    LOG_S(ERROR) << message;
                    throw UnImplementedException(message);
                }
                default: {
                    std::string message = "Missing case for this TypeKind in switch";
                    LOG_S(ERROR) << message;
                    throw NoSuchTypeException(message);
                }
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
        if (!isInitializedStruct && !curStruct.name.empty() && !anonymous) {
            // init by memory copy
            entryValue = PrinterUtils::convertBytesToStruct(
                    curStruct.name,
                    fixedArrayView(rawData,
                                   types::Type::createSimpleTypeFromName("utbot_byte"),
                                   curStruct.name,
                                   curStruct.size,
                                   offsetInBits, objects, initReferences, testingMethod)->getEntryValue(nullptr));
            isInitializedStruct = true;
            dirtyInitializedStruct = false;
        }
        if (!isInitializedStruct) {
            dirtyInitializedStruct = false;
        }
        return std::make_shared<StructValueView>(curStruct, subViews, entryValue,
                                                 anonymous, isInitializedStruct, dirtyInitializedStruct,
                                                 fieldIndexToInitUnion);
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
        template<typename T>
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
        for (auto &[testMethod, testCases]: batch) {
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
        size_t indField = std::upper_bound(structInfo.fields.begin(), structInfo.fields.end(), offsetInBits,
                                           [](int offset, const Field &field) {
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
                                       std::queue<JsonIndAndParam> &order) {
        auto it = std::find_if(objects.begin(), objects.end(),
                               [paramName](const UTBotKTestObject &obj) { return obj.name == paramName; });
        if (it != objects.end()) {
            size_t jsonInd = it - objects.begin();
            visited[jsonInd] = true;
//        usages[jsonInd] = types::PointerUsage::PARAMETER;
//        Tests::MethodParam param = { paramType.isObjectPointer() && !paramType.isPointerToPointer()
//                                         ? paramType.baseTypeObj()
//                                         : paramType,
//                                    paramName, std::nullopt };
            Tests::MethodParam param = {paramType, paramName, std::nullopt};
            order.emplace(jsonInd, param, paramValue);
            return;
        }
        std::string message = "Don't find object " + paramName + " in objects array";
        LOG_S(WARNING) << message;
    }

//    bool KTestObjectParser::pointToStruct(const types::Type &pointerType,
//                                          const UTBotKTestObject &goal) const {
//        // In different situations we may point on the whole struct or on the field with assignment 0
//        size_t fieldSizeInBits = typesHandler.typeSize(pointerType.baseTypeObj(1));
//        size_t pointerVarSizeInBytes = goal.bytes.size();
//        return SizeUtils::bytesToBits(pointerVarSizeInBytes) == fieldSizeInBits;
//    }

    void KTestObjectParser::assignTypeUnnamedVar(
            Tests::MethodTestCase &testCase,
            const Tests::MethodDescription &methodDescription,
            std::vector<std::optional<Tests::TypeAndVarName>> &typeAndName) {
        std::queue<JsonIndAndParam> order;
        std::vector<bool> visited(testCase.kleeObjects.size(), false);
        for (size_t paramInd = 0; paramInd < testCase.paramValues.size(); paramInd++) {
            addToOrder(testCase.kleeObjects, methodDescription.params[paramInd].name,
                       methodDescription.params[paramInd].type, testCase.paramValues[paramInd], visited, order);
        }

        while (!order.empty()) {
            auto curType = order.front();
            order.pop();
            std::string paramName = testCase.kleeObjects[curType.jsonInd].name;
            types::Type paramType = curType.param.type;
            typeAndName[curType.jsonInd] = {paramType, paramName};

            if (testCase.kleeObjects[curType.jsonInd].is_lazy) {
                std::shared_ptr<AbstractValueView> testParamView = testPreValueView(
                        testCase.kleeObjects[curType.jsonInd],
                        paramType,
                        paramName,
                        testCase,
                        methodDescription);
                LOG_S(MAX) << "Fetch lazy object: " << paramName << " = " << testParamView->getEntryValue(nullptr);
                curType.paramValue.lazyParams.emplace_back(paramType, paramName, std::nullopt);
                curType.paramValue.lazyValues.emplace_back(paramName, std::nullopt, testParamView);
            }
            //TODO add post
            for (auto const &[offset, indObj, indexOffset]: testCase.kleeObjects[curType.jsonInd].preRaw.pointers) {
                if (!visited[indObj]) {
//                if (indexOffset != 0) {
//                    continue;
//                }

                    Tests::TypeAndVarName typeAndName = {paramType, ""};
//                size_t offsetInStruct = getOffsetInStruct(typeAndName, SizeUtils::bytesToBits(offset)/*, usages[indObj]*/);
                    size_t offsetInStruct = SizeUtils::bytesToBits(offset);
                    types::Type fieldType = traverseLazy(typeAndName.type, offsetInStruct).type;

//                if (!pointToStruct(fieldType, testCase.objects[indObj])) {
//                    continue;
//                }

                    Tests::MethodParam param(fieldType.arrayClone(), "", std::nullopt);
                    order.emplace(indObj, param, curType.paramValue);
                    visited[indObj] = true;
//                usages[indObj] = types::PointerUsage::PARAMETER;
                }
            }
        }

        visited = std::vector<bool>(testCase.kleeObjects.size(), false);
        for (size_t paramInd = 0; paramInd < testCase.paramPostValues.size(); paramInd++) {
            addToOrder(testCase.kleeObjects, methodDescription.params[paramInd].name,
                       methodDescription.params[paramInd].type, testCase.paramPostValues[paramInd], visited, order);
        }
        addToOrder(testCase.kleeObjects, PrinterUtils::ACTUAL, methodDescription.returnType,
                   testCase.returnValue, visited, order);

        while (!order.empty()) {
            auto curType = order.front();
            order.pop();
            std::string paramName = testCase.kleeObjects[curType.jsonInd].name;
            std::string expectedParamName = KleeUtils::postSymbolicVariable(paramName);
            types::Type paramType = curType.param.type;
            typeAndName[curType.jsonInd] = {paramType, paramName};

            if (testCase.kleeObjects[curType.jsonInd].is_lazy) {


                std::shared_ptr<AbstractValueView> testParamViewPost = testPostValueView(
                        testCase.kleeObjects[curType.jsonInd],
                        paramType,
                        expectedParamName,
                        testCase,
                        methodDescription);
                LOG_S(MAX)
                << "Fetch lazy object: " << expectedParamName << " = " << testParamViewPost->getEntryValue(nullptr);
                curType.paramValue.lazyParams.emplace_back(paramType, expectedParamName, std::nullopt);
                curType.paramValue.lazyValues.emplace_back(expectedParamName, std::nullopt, testParamViewPost);
            }
            //TODO add post
            for (auto const &[offset, indObj, indexOffset]: testCase.kleeObjects[curType.jsonInd].postRaw.pointers) {
                if (!visited[indObj]) {
                    Tests::TypeAndVarName typeAndName = {paramType, ""};
                    size_t offsetInStruct = SizeUtils::bytesToBits(offset);
                    types::Type fieldType = traverseLazy(typeAndName.type, offsetInStruct).type;

                    Tests::MethodParam param(fieldType.arrayClone(), "", std::nullopt);
                    order.emplace(indObj, param, curType.paramValue);
                    visited[indObj] = true;
                }
            }
        }
    }

    Tests::TypeAndVarName KTestObjectParser::traverseLazy(const types::Type &curVarType,
                                                          size_t offsetInBits,
                                                          const std::string &curVarName) const {
        switch (typesHandler.getTypeKind(curVarType)) {
            case TypeKind::STRUCT_LIKE: {
                const types::StructInfo &structInfo = typesHandler.getStructInfo(curVarType);
                size_t indField = findFieldIndex(structInfo, offsetInBits);
                const types::Field &next = structInfo.fields[indField];
                return traverseLazy(next.type, offsetInBits - next.offset,
                                    PrinterUtils::getFieldAccess(curVarName, next));
            }
            case TypeKind::ARRAY: {
//            LOG_IF_S(ERROR, offsetInBits != 0) << "Offset not zero" << offsetInBits;
                //TODO change name constructor
                const types::Type subType = curVarType.baseTypeObj(1);
                size_t offsetInArray = (offsetInBits >> 3) / typesHandler.getPointerSize();
                size_t newOffset = offsetInBits - offsetInArray * typesHandler.getPointerSize();
                std::string varname = StringUtils::stringFormat("%s[%d]", curVarName, offsetInArray);
                return traverseLazy(subType, newOffset, varname);
            }
            case TypeKind::OBJECT_POINTER:
            case TypeKind::PRIMITIVE: {
                return {curVarType, curVarName};
            }
            case TypeKind::ENUM:
            case TypeKind::FUNCTION_POINTER:
            case TypeKind::UNKNOWN:
            default: {
                std::string message =
                        "Unsupported type in lazy initialization BFS: " + curVarType.typeName();
                LOG_S(ERROR) << message;
                throw NoSuchTypeException(message);
            }
        }
    }

//size_t KTestObjectParser::getOffsetInStruct(Tests::TypeAndVarName &objTypeAndName,
//                                            size_t offsetInBits/*,
//                                            types::PointerUsage usage*/) const {
//    if (!objTypeAndName.type.isPointerToPointer() /* || usage != types::PointerUsage::PARAMETER*/) {
//        return offsetInBits;
//    }
//    //TODO
//    std::vector<size_t> sizes = {1}; //objTypeAndName.type.arraysSizes(/*usage*/);
//    objTypeAndName.type = objTypeAndName.type.baseTypeObj();
//    size_t sizeInBits = typesHandler.typeSize(objTypeAndName.type);
//    size_t offset = offsetInBits / sizeInBits;
//    PrinterUtils::appendIndicesToVarName(objTypeAndName.varName, sizes, offset);
//    if (objTypeAndName.type.isConstQualifiedValue()) {
//        PrinterUtils::appendConstCast(objTypeAndName.varName);
//    }
//    offsetInBits %= sizeInBits;
//    return offsetInBits;
//}

    void KTestObjectParser::assignTypeStubVar(Tests::MethodTestCase &testCase,
                                              const Tests::MethodDescription &methodDescription) {
        for (auto const &obj: testCase.kleeObjects) {
            std::optional<std::shared_ptr<FunctionInfo>>
                    maybeFunctionInfo = methodDescription.stubsParamStorage->getFunctionInfoByKTestObjectName(obj.name);
            if (maybeFunctionInfo.has_value()) {
                types::Type stubType = types::Type::createArray(maybeFunctionInfo.value()->returnType);
                std::shared_ptr<AbstractValueView> stubView =
                        testPreValueView(obj, stubType, obj.name, testCase, methodDescription);
                testCase.stubParamValues.emplace_back(obj.name, 0, stubView);
                testCase.stubParamTypes.emplace_back(stubType, obj.name, std::nullopt);
            }
        }
    }

    void KTestObjectParser::assignAllLazyPointers(
            Tests::MethodTestCase &testCase,
            const std::vector<std::optional<Tests::TypeAndVarName>> &objTypeAndName) const {
        for (size_t ind = 0; ind < testCase.kleeObjects.size(); ind++) {
            const auto &object = testCase.kleeObjects[ind];
            if (!objTypeAndName[ind].has_value()) {
                continue;
            }
            //TODO add post
            for (const auto &pointer: object.preRaw.pointers) {

                Tests::TypeAndVarName typeAndName = objTypeAndName[ind].value();
//            size_t offset = getOffsetInStruct(typeAndName,
//                                              SizeUtils::bytesToBits(pointer.offset)/*,
//                                              usages[ind]*/);
                size_t offset = SizeUtils::bytesToBits(pointer.offset);
                Tests::TypeAndVarName fromPtr =
                        traverseLazy(typeAndName.type, offset, typeAndName.varName);
                if (!objTypeAndName[pointer.indexOfObject].has_value()) {
                    continue;
                }

//            std::string toPtrName;
                Tests::TypeAndVarName pointerTypeAndName = objTypeAndName[pointer.indexOfObject].value();
//            size_t indexOffset = getOffsetInStruct(pointerTypeAndName,
//                                                   SizeUtils::bytesToBits(pointer.indexOffset)/*,
//                                                   usages[pointer.index]*/);
//            if (indexOffset == 0 &&
//                pointToStruct(fromPtr.type, testCase.objects[pointer.index])) {
//                toPtrName = pointerTypeAndName.varName;
//            } else {
//                toPtrName = traverseLazy(pointerTypeAndName.type, indexOffset,
//                                         pointerTypeAndName.varName).varName;
//            }
                std::string toPtrName = pointerTypeAndName.varName;

                testCase.lazyReferences.emplace_back(
                        fromPtr.varName, toPtrName,
                        PrinterUtils::getTypeForinitializePointerToVar(fromPtr.type.baseType(),
                                                                       fromPtr.type.getDimension(),
                                                                       fromPtr.type.isConstQualifiedValue()));
            }
        }
    }

    void KTestObjectParser::parseTestCases(const UTBotKTestList &cases,
                                           bool filterByLineFlag,
                                           Tests::MethodDescription &methodDescription,
                                           const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
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
        for (const auto &case_: cases) {
            try {
                std::stringstream traceStream;
                traceStream << "Test case #" << (++caseCounter) << ":\n";
                std::string suiteName = getSuiteName(case_.status, lineInfo);
                Tests::MethodTestCase testCase;
                testCase.testIndex = testIndex;
                testCase.suiteName = suiteName;
                std::vector<Tests::TestCaseParamValue> paramValues;

                Tests::TestCaseValues testCaseValues = parseTestCaseParameters(case_,
                                                                               methodDescription,
                                                                               methodNameToReturnTypeMap,
                                                                               traceStream);

                size_t size = case_.objects.size();
                bool isVoidOrFunctionPointer = types::TypesHandler::skipTypeInReturn(methodDescription.returnType);
                if ((isVoidOrFunctionPointer && size > 0) || (!isVoidOrFunctionPointer && size > 1)) {
                    std::swap(testCase.paramValues, testCaseValues.paramValues);
                } else {
                    // If all the data characters are not printable the case is skipped
                    continue;
                }
                std::swap(testCase.classPreValues, testCaseValues.classPreValues);
                std::swap(testCase.classPostValues, testCaseValues.classPostValues);
                std::swap(testCase.globalPreValues, testCaseValues.globalPreValues);
                std::swap(testCase.globalPostValues, testCaseValues.globalPostValues);
                std::swap(testCase.paramPostValues, testCaseValues.paramPostValues);
                std::swap(testCase.stubValuesTypes, testCaseValues.stubValuesTypes);
                std::swap(testCase.stubValues, testCaseValues.stubValues);
                std::swap(testCase.stdinValue, testCaseValues.stdinValue);
                std::swap(testCase.filesValues, testCaseValues.filesValues);
                std::swap(testCase.kleeObjects, testCaseValues.kleeObjects);
                std::swap(testCase.lazyReferences, testCaseValues.lazyReferences);
                std::swap(testCase.lazyReferencesPost, testCaseValues.lazyReferencesPost);

                testCase.errorDescriptors = case_.errorDescriptors;
                testCase.errorInfo = testCaseValues.errorInfo;

                if (filterByLineFlag) {
                    auto view = testCaseValues.kleePathFlagSymbolicValue.view;
                    if (!view || view->getEntryValue(nullptr) != "1") {
                        continue;
                    }
                }
                auto const &predicateInfo = lineInfo ? lineInfo->predicateInfo : std::nullopt;
                if (predicateInfo.has_value() &&
                    !predicateMatch(testCaseValues.returnValue.view->getEntryValue(nullptr),
                                    predicateInfo.value())) {
                    continue;
                }

                if (predicateInfo.has_value() && predicateInfo->type != testsgen::STRING) {
                    testCase.returnValue.view = std::make_shared<PrimitiveValueView>(
                            PrinterUtils::wrapUserValue(predicateInfo->type, predicateInfo->returnValue));
                } else {
                    testCase.returnValue.view = testCaseValues.returnValue.view;
                }

                if (methodDescription.returnType.isObjectPointer() && !methodDescription.returnType.maybeArray
                    && testCaseValues.functionReturnNotNullValue.view &&
                    testCaseValues.functionReturnNotNullValue.view->getEntryValue(nullptr) == "0") {
                    testCase.returnValue.view = std::make_shared<PrimitiveValueView>(PrinterUtils::C_NULL);
                }
                traceStream << "\treturn: " << testCase.returnValue.view->getEntryValue(nullptr);
                LOG_S(MAX) << traceStream.str();

                std::vector<std::optional<Tests::TypeAndVarName>> objectsValues(testCase.kleeObjects.size());
                assignTypeUnnamedVar(testCase, methodDescription, objectsValues);
                assignTypeStubVar(testCase, methodDescription);
//                assignAllLazyPointers(testCase, objectsValues);

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

    std::vector<UTBotKTestObject>::const_iterator
    KTestObjectParser::getKleeParam(const std::vector<UTBotKTestObject> &objects, const std::string name) {
        return std::find_if(objects.begin(), objects.end(),
                            [&](const UTBotKTestObject &param) { return param.name == name; });
    }

    UTBotKTestObject KTestObjectParser::getKleeParamOrThrow(const std::vector<UTBotKTestObject> &objects,
                                                            const std::string &name) {
        const auto kleeParam = getKleeParam(objects, name);
        if (kleeParam == objects.end()) {
            std::string message = "Parameter \'" + name + "\' not found.";
            LOG_S(ERROR) << message;
            throw UnImplementedException(message);
        }

        return *kleeParam;
    }

    Tests::TestCaseValues KTestObjectParser::parseTestCaseParameters(
            const UTBotKTest &ktest,
            Tests::MethodDescription &methodDescription,
            const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
            std::stringstream &traceStream) {

        Tests::TestCaseValues testCaseValues;
        testCaseValues.kleeObjects = ktest.objects;
        testCaseValues.errorInfo = ktest.errorInfo;

        for (size_t i = 0; i < testCaseValues.kleeObjects.size(); ++i) {
            if (testCaseValues.kleeObjects[i].name != LAZYNAME) {
                continue;
            }
            testCaseValues.kleeObjects[i].name = PrinterUtils::generateNewVar(i);
        }

        if (methodDescription.isClassMethod()) {
            auto methodParam = methodDescription.classObj.value();
            std::shared_ptr<AbstractValueView> testParamView;
            getTestParamView(methodDescription, testCaseValues, methodParam, testParamView);
            testCaseValues.classPreValues = {methodParam.name, methodParam.alignment, testParamView};
            processClassPostValue(testCaseValues, methodParam, testCaseValues.kleeObjects);
        }

        for (auto &methodParam: methodDescription.params) {
            {
                std::shared_ptr<AbstractValueView> testParamView;
                if (!methodParam.type.isFilePointer()) {
                    getTestParamView(methodDescription, testCaseValues, methodParam, testParamView);
                } else {
                    testParamView = std::shared_ptr<AbstractValueView>(new JustValueView("FILE_PTR"));
                }
                testCaseValues.paramValues.emplace_back(methodParam.name, methodParam.alignment,
                                                        testParamView);
            }

            if (methodParam.isChangeable()) {
                processParamPostValue(testCaseValues, methodParam, testCaseValues.kleeObjects);
            }
        }
        for (const auto &globalParam: methodDescription.globalParams) {
            processGlobalParamPreValue(testCaseValues, globalParam, testCaseValues.kleeObjects);
            processGlobalParamPostValue(testCaseValues, globalParam, testCaseValues.kleeObjects);
        }

        if (Paths::getSourceLanguage(sourceFilePath) == utbot::Language::C) {
            processSymbolicStdin(testCaseValues, testCaseValues.kleeObjects);
            processSymbolicFiles(testCaseValues, testCaseValues.kleeObjects);
        }

        processStubParamValue(methodDescription, testCaseValues, methodNameToReturnTypeMap, testCaseValues.kleeObjects);
        if (!types::TypesHandler::skipTypeInReturn(methodDescription.returnType)) {
            const auto kleeResParam = getKleeParamOrThrow(testCaseValues.kleeObjects, PrinterUtils::ACTUAL);
            auto paramType = methodDescription.returnType;
            const auto testReturnView = testPostValueView(
                    kleeResParam, paramType, PrinterUtils::ACTUAL,
                    testCaseValues, methodDescription);
            testCaseValues.returnValue = {
                    PrinterUtils::ACTUAL,
                    types::TypesHandler::isObjectPointerType(methodDescription.returnType),
                    testReturnView
            };
        } else {
            testCaseValues.returnValue = {PrinterUtils::ACTUAL, false,
                                          std::make_shared<VoidValueView>()};
        }

        const auto kleePathFlagIterator = getKleeParam(testCaseValues.kleeObjects, KLEE_PATH_FLAG);
        const auto kleePathFlagSymbolicIterator = getKleeParam(testCaseValues.kleeObjects, KLEE_PATH_FLAG_SYMBOLIC);
        if (kleePathFlagSymbolicIterator != testCaseValues.kleeObjects.end()) {
            const auto kleePathFlagSymbolicView = testPreValueView(
                    *kleePathFlagSymbolicIterator, types::Type::intType(), KLEE_PATH_FLAG_SYMBOLIC,
                    testCaseValues);
            testCaseValues.kleePathFlagSymbolicValue = {KLEE_PATH_FLAG_SYMBOLIC, false,
                                                        kleePathFlagSymbolicView};
        }
        const auto functionReturnNotNullIterator = getKleeParam(testCaseValues.kleeObjects,
                                                                KleeUtils::NOT_NULL_VARIABLE_NAME);
        if (functionReturnNotNullIterator != testCaseValues.kleeObjects.end()) {
            const auto functionReturnNotNullView = testPreValueView(
                    *functionReturnNotNullIterator, types::Type::intType(), KleeUtils::NOT_NULL_VARIABLE_NAME,
                    testCaseValues);
            testCaseValues.functionReturnNotNullValue = {KleeUtils::NOT_NULL_VARIABLE_NAME, false,
                                                         functionReturnNotNullView};
        }
        return testCaseValues;
    }


    const UTBotKTestObject emptyKleeObject = {"", {}, {}, {}, {}, 0, false};

    void KTestObjectParser::getTestParamView(const Tests::MethodDescription &methodDescription,
                                             Tests::TestCaseValues &testCaseValues,
                                             const Tests::MethodParam &methodParam,
                                             std::shared_ptr<AbstractValueView> &testParamView) {
//    const auto usage = types::PointerUsage::PARAMETER;
        types::Type paramType = methodParam.type.arrayCloneMultiDim(/*usage*/);
        auto type = typesHandler.getReturnTypeToCheck(paramType);

        if (CollectionUtils::containsKey(methodDescription.functionPointers, methodParam.name)) {
            testParamView = testPreValueView(emptyKleeObject, type, methodParam.name,
                                             testCaseValues, methodDescription);
        } else {
            const auto kleeParam = getKleeParamOrThrow(testCaseValues.kleeObjects, methodParam.name);
            testParamView = testPreValueView(kleeParam, type, methodParam.name,
                                             testCaseValues, methodDescription);
        }
    }

    void KTestObjectParser::processGlobalParamPreValue(Tests::TestCaseValues &testCaseValues,
                                                       const Tests::MethodParam &globalParam,
                                                       const std::vector<UTBotKTestObject> &objects) {
        std::string kleeParamName = globalParam.name;
        auto kleeParam = getKleeParamOrThrow(objects, kleeParamName);
        auto testParamView = testPreValueView(
                kleeParam, globalParam.type, globalParam.name,
                testCaseValues);
        testCaseValues.globalPreValues.emplace_back(globalParam.name, globalParam.alignment,
                                                    testParamView);
    }

    void KTestObjectParser::processSymbolicStdin(Tests::TestCaseValues &testCaseValues,
                                                 const std::vector<UTBotKTestObject> &objects) {
        auto &&read = getKleeParamOrThrow(objects, KleeUtils::STDIN_READ_NAME);
        std::string &&view = testPreValueView(read, types::Type::longlongType(), KleeUtils::STDIN_READ_NAME,
                                              testCaseValues)
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
            auto stdinBuffer = getKleeParamOrThrow(objects, KleeUtils::STDIN_NAME);
            auto testParamView = stringLiteralView(stdinBuffer.preRaw.bytes, usedStdinBytesCount);
            testCaseValues.stdinValue = Tests::TestCaseParamValue(types::Type::getStdinParamName(),
                                                                  std::nullopt, testParamView);
        }
    }

    void KTestObjectParser::processSymbolicFiles(Tests::TestCaseValues &testCaseValues,
                                                 const std::vector<UTBotKTestObject> &objects) {
        std::vector<Tests::FileInfo> filesValues(types::Type::symFilesCount);
        int fileIndex = 0;
        for (char fileName = 'A'; fileName < 'A' + types::Type::symFilesCount;
             fileName++, fileIndex++) {
            std::string readBytesName = PrinterUtils::getFileReadBytesParamKTestJSON(fileName);
            auto &&readBytes = getKleeParamOrThrow(objects, readBytesName);
            filesValues[fileIndex].readBytes =
                    std::stoi(testPreValueView(readBytes, types::Type::longlongType(), readBytesName,
                                               testCaseValues)
                                      ->getEntryValue(nullptr));

            std::string writeBytesName = PrinterUtils::getFileWriteBytesParamKTestJSON(fileName);
            auto &&writeBytes = getKleeParamOrThrow(objects, writeBytesName);
            filesValues[fileIndex].writeBytes =
                    std::stoi(testPreValueView(writeBytes, types::Type::longlongType(), writeBytesName,
                                               testCaseValues)
                                      ->getEntryValue(nullptr));

            auto fileBuffer = getKleeParamOrThrow(objects, PrinterUtils::getFileParamKTestJSON(fileName));
            filesValues[fileIndex].data =
                    stringLiteralView(fileBuffer.preRaw.bytes, filesValues[fileIndex].readBytes)
                            ->getEntryValue(nullptr);
        }
        testCaseValues.filesValues = filesValues;
    }

    void KTestObjectParser::processGlobalParamPostValue(Tests::TestCaseValues &testCaseValues,
                                                        const Tests::MethodParam &globalParam,
                                                        std::vector<UTBotKTestObject> &objects) {
        auto kleeParam = getKleeParamOrThrow(objects, globalParam.name);
        auto testParamView = testPostValueView(kleeParam, globalParam.type, globalParam.name, testCaseValues);
        testCaseValues.globalPostValues.emplace_back(globalParam.name, globalParam.alignment, testParamView);
    }

    void KTestObjectParser::processClassPostValue(Tests::TestCaseValues &testCaseValues,
                                                  const Tests::MethodParam &param,
                                                  std::vector<UTBotKTestObject> &objects) {
        auto kleeParam = getKleeParamOrThrow(objects, param.name);
        auto testParamView = testPostValueView(kleeParam, param.type, param.name, testCaseValues);
        testCaseValues.classPostValues = {param.name, param.alignment, testParamView};
    }

    void KTestObjectParser::processParamPostValue(Tests::TestCaseValues &testCaseValues,
                                                  const Tests::MethodParam &param,
                                                  std::vector<UTBotKTestObject> &objects) {
        auto kleeParam = getKleeParamOrThrow(objects, param.name);
        auto testParamView = testPostValueView(kleeParam, param.type, param.name, testCaseValues);
        testCaseValues.paramPostValues.emplace_back(param.name, param.alignment, testParamView);
    }

    void KTestObjectParser::processStubParamValue(
            const Tests::MethodDescription &methodDescription,
            Tests::TestCaseValues &testCaseValues,
            const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
            std::vector<UTBotKTestObject> &objects) {
        for (const auto &ktestObject: objects) {
            auto maybeFunctionInfo = methodDescription.stubsStorage->getFunctionInfoByKTestObjectName(
                    ktestObject.name);
            if (maybeFunctionInfo.has_value()) {
                types::Type stubType = types::Type::createArray(maybeFunctionInfo.value()->returnType);
                auto testParamView =
                        testPreValueView(ktestObject, stubType, ktestObject.name,
                                         testCaseValues);
                testCaseValues.stubValues.emplace_back(ktestObject.name, 0, testParamView);
                testCaseValues.stubValuesTypes.emplace_back(stubType, ktestObject.name, std::nullopt);
            }
        }
    }

    std::shared_ptr<AbstractValueView> KTestObjectParser::testValueView(
            const UTBotKTestObject::RawData &rawData,
            const types::Type &paramType,
            const std::string &paramName,
            const std::vector<UTBotKTestObject> &objects,
            std::vector<InitReference> &initReferences,
            const std::optional<const Tests::MethodDescription> &testingMethod) {

        const size_t sizeInBits = SizeUtils::bytesToBits(rawData.bytes.size());

        switch (typesHandler.getTypeKind(paramType)) {
            case TypeKind::STRUCT_LIKE:
                return structView(rawData, typesHandler.getStructInfo(paramType), paramName, objects, initReferences,
                                  testingMethod, 0, false);
            case TypeKind::ENUM:
                return enumView(rawData, typesHandler.getEnumInfo(paramType), 0, sizeInBits);
            case TypeKind::PRIMITIVE:
                return primitiveView(rawData, paramType.baseTypeObj(), 0, sizeInBits);
            case TypeKind::OBJECT_POINTER: {
                return getLazyPointerView(paramName, paramType, !rawData.pointers.empty(), objects, initReferences,
                                          rawData, 0);
            }
            case TypeKind::FUNCTION_POINTER:
                if (!testingMethod.has_value()) {
                    return functionPointerView(std::nullopt, "", paramName);
                }
                return functionPointerView(testingMethod->getClassTypeName(), testingMethod->name, paramName);
            case TypeKind::ARRAY:
                return fixedArrayView(rawData, paramType, paramName, sizeInBits, 0, objects, initReferences,
                                      testingMethod);
            case TypeKind::UNKNOWN: {
                std::string message = "No such type";
                LOG_S(ERROR) << message;
                throw UnImplementedException(message);
            }
            default: {
                std::string message = "Missing case for this TypeKind in switch";
                LOG_S(ERROR) << message;
                throw NoSuchTypeException(message);
            }
        }
    }

    std::shared_ptr<AbstractValueView> KTestObjectParser::testPreValueView(
            const tests::UTBotKTestObject &kleeParam,
            const types::Type &paramType,
            const std::string &paramName,
            Tests::TestCaseValues &testCaseValues,
            const std::optional<const Tests::MethodDescription> &testingMethod) {
        return testValueView(kleeParam.preRaw, paramType, paramName, testCaseValues.kleeObjects,
                             testCaseValues.lazyReferences, testingMethod);
    }

    std::shared_ptr<AbstractValueView> KTestObjectParser::testPostValueView(
            const UTBotKTestObject &kleeParam,
            const types::Type &paramType,
            const std::string &paramName,
            Tests::TestCaseValues &testCaseValues,
            const std::optional<const Tests::MethodDescription> &testingMethod) {
        return testValueView(kleeParam.postRaw, paramType, paramName, testCaseValues.kleeObjects,
                             testCaseValues.lazyReferencesPost, testingMethod);
    }

    std::shared_ptr<AbstractValueView>
    KTestObjectParser::getLazyPointerView(const std::string &name,
                                          const Type &paramType,
                                          bool lazyPointer,
                                          const std::vector<UTBotKTestObject> &objects,
                                          std::vector<InitReference> &initReferences,
                                          const UTBotKTestObject::RawData &rawData,
                                          const size_t offset) const {
        const std::string res = readBytesAsValueForType(rawData.bytes, PointerWidthType, offset,
                                                        PointerWidthSizeInBits);
        const size_t ptr = std::stoull(res);

        size_t shift = 0;
        for (const auto &i: rawData.pointers) {
            if (i.offset == offset) {
                shift = i.indexOffset;
            }
        }

        auto ptrElement = std::find_if(objects.begin(), objects.end(),
                                       [ptr, shift](const UTBotKTestObject &object) {
                                           return object.address + shift == ptr;
                                       });
        if (ptrElement != objects.end()) {
            std::string ptrElementName = rawData.isPost ? KleeUtils::postSymbolicVariable(ptrElement->name)
                                                        : ptrElement->name;
            initReferences.emplace_back(
                    name, ptrElementName,
                    PrinterUtils::getTypeForinitializePointerToVar(paramType.baseType(),
                                                                   paramType.getDimension(),
                                                                   paramType.isConstQualifiedValue()));
        }
//    if (lazyPointer || ptr_element != objects.end()) {
//            res = PrinterUtils::C_NULL;
//    }
        return std::make_shared<JustValueView>(
                PrinterUtils::initializePointer(paramType.baseType(), res, paramType.getDimension(),
                                                paramType.isConstQualifiedValue()));
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
        for (const auto &parameter: methodDescription.params) {
            signatureHash += parameter.type.typeName();
        }
        return std::hash<std::string>()(signatureHash);
    }

    TestMethod::TestMethod(std::string methodName, fs::path bitcodeFile, fs::path sourceFilename, bool is32)
            : methodName(std::move(methodName)), bitcodeFilePath(std::move(bitcodeFile)),
              sourceFilePath(std::move(sourceFilename)), is32bits(is32) {}

    bool TestMethod::operator==(const TestMethod &rhs) const {
        return std::tie(methodName, bitcodeFilePath, sourceFilePath, is32bits)
               == std::tie(rhs.methodName, rhs.bitcodeFilePath, rhs.sourceFilePath, rhs.is32bits);
    }

    bool TestMethod::operator!=(const TestMethod &rhs) const {
        return !(rhs == *this);
    }

    UTBotKTestObject::UTBotKTestObject(std::string name,
                                       std::vector<char> bytes,
                                       std::vector<char> finalBytes,
                                       std::vector<Pointer> pointers,
                                       std::vector<Pointer> finalPointers,
                                       size_t address,
                                       bool is_lazy)
            : name(std::move(name)),
              preRaw({std::move(bytes), std::move(pointers), false}),
              postRaw({std::move(finalBytes), std::move(finalPointers), true}),
              address(address),
              is_lazy(is_lazy) {
    }


    UTBotKTestObject UTBotKTestObject::fromKTest(const KTestObject &kTestObject) {
        std::vector<char> bytes = kTestObject.content.bytes != nullptr ?
                                  std::vector<char>(
                                          kTestObject.content.bytes,
                                          kTestObject.content.bytes + kTestObject.content.numBytes
                                  ) :
                                  std::vector<char>(0);
        std::vector<char> finalBytes = kTestObject.content.finalBytes != nullptr ?
                                       std::vector<char>(
                                               kTestObject.content.finalBytes,
                                               kTestObject.content.finalBytes + kTestObject.content.numBytes
                                       ) :
                                       std::vector<char>(0);

        return UTBotKTestObject(kTestObject.name,
                                std::move(bytes),
                                std::move(finalBytes),
                                {kTestObject.content.pointers,
                                 kTestObject.content.pointers + kTestObject.content.numPointers},
                                {kTestObject.content.finalPointers,
                                 kTestObject.content.finalPointers + kTestObject.content.numFinalPointers},
                                kTestObject.address,
                                isUnnamed(kTestObject.name));
    }

    bool isUnnamed(char *name) {
        return strcmp(name, LAZYNAME.c_str()) == 0;
    }

    bool Tests::MethodTestCase::isError() const {
        return suiteName == ERROR_SUITE_NAME;
    }

    bool Tests::TypeAndVarName::operator<(const Tests::TypeAndVarName &other) const {
        return varName < other.varName || (varName == other.varName && type.mTypeName() < other.type.mTypeName());
    }
} // tests
