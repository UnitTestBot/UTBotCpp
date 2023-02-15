#include "KleeConstraintsPrinter.h"

#include "utils/PrinterUtils.h"
#include "utils/StubsUtils.h"
#include "exceptions/UnImplementedException.h"

#include "loguru.h"

#include <cstddef>

using namespace types;
using printer::KleeConstraintsPrinter;

printer::KleeConstraintsPrinter::KleeConstraintsPrinter(const types::TypesHandler *typesHandler, utbot::Language srcLanguage)
    : Printer(srcLanguage), typesHandler(typesHandler) {}

printer::KleeConstraintsPrinter::Stream
KleeConstraintsPrinter::genConstraints(const std::string &name, const types::Type& type, const std::vector<std::string>& names) {
    ConstraintsState state = { "&" + name, name, type, true };
    auto paramType = type;
    if (type.maybeJustPointer()) {
        state.curType = paramType = type.baseTypeObj();
    }
    switch (typesHandler->getTypeKind(paramType)) {
        case TypeKind::OBJECT_POINTER:
        case TypeKind::ARRAY:
            state = { name, name, paramType, state.endString };
            genConstraintsForPointerOrArray(state);
            break;
        case TypeKind::STRUCT_LIKE:
            genConstraintsForStruct(state);
            break;
        case TypeKind::ENUM:
            genConstraintsForEnum(state);
            break;
        default:
            genConstraintsForPrimitive(state, names);
    }

    return ss;
}

printer::KleeConstraintsPrinter::Stream
KleeConstraintsPrinter::genConstraints(const Tests::MethodParam &param, const std::vector<std::string>& names) {
    return genConstraints(param.name, param.type, names);
}

void KleeConstraintsPrinter::genConstraintsForPrimitive(const ConstraintsState &state, const std::vector<std::string>& names) {
    const auto &cons = cexConstraints(state.curElement, state.curType, names);
    if (!cons.empty()) {
        strFunctionCall(PrinterUtils::KLEE_PREFER_CEX, { state.paramName, cons });
    } else {
        noConstraints(state.curElement);
    }
}

void KleeConstraintsPrinter::noConstraints(const std::string &cause) {
    ss << LINE_INDENT() << "// No constraints for " << cause << NL;
}

void KleeConstraintsPrinter::genConstraintsForEnum(const ConstraintsState &state) {
    types::EnumInfo enumInfo = typesHandler->getEnumInfo(state.curType);

    std::stringstream _ss;
    for (auto it = enumInfo.namesToEntries.begin(); it != enumInfo.namesToEntries.end(); ++it) {
        _ss << state.curElement << PrinterUtils::EQ_OPERATOR << it->second.name;
        if (std::next(it) != enumInfo.namesToEntries.end()) {
            _ss << " | ";
        }
    }
    strFunctionCall(PrinterUtils::KLEE_ASSUME, { _ss.str() });
}

void KleeConstraintsPrinter::genConstraintsForPointerOrArray(const ConstraintsState &state) {
    auto sizes = state.curType.arraysSizes(types::PointerUsage::PARAMETER);
    genConstraintsForMultiPointerOrArray(state, sizes);
}

void KleeConstraintsPrinter::genConstraintsForMultiPointerOrArray(const ConstraintsState &state,
                                                                  std::vector<size_t> sizes) {
    const types::Type baseType = state.curType.baseTypeObj();
    bool assignPointersToNull = state.curType.isTypeContainsPointer() && state.depth > 0;
    if (assignPointersToNull) {
        int pointerIndex = state.curType.indexOfFirstPointerInTypeKinds();
        sizes = std::vector<size_t>(sizes.begin(), sizes.begin() + pointerIndex);
    }
    const auto indexes = printForLoopsAndReturnLoopIterators(sizes);
    std::string element = constrMultiIndex(state.curElement, indexes);

    if (state.endString && types::TypesHandler::isCharacterType(baseType) &&
        TypesHandler::isCStringType(state.curType)) {
        std::vector<std::string> charSizes(indexes.begin(), indexes.end() - 1);
        const auto charElement = constrMultiIndex(state.curElement, charSizes);
        ss << LINE_INDENT() << "if (" << indexes.back() << PrinterUtils::EQ_OPERATOR << sizes.back() - 1 << ")" << LB();
        ss << LINE_INDENT() << PrinterUtils::KLEE_ASSUME << "(" << charElement << "[" << sizes.back() - 1 << "]" << PrinterUtils::EQ_OPERATOR << "'\\0'" << ")" << SCNL;
        ss << LINE_INDENT() << "break" << SCNL;
        ss << RB();
    }

    ConstraintsState newState = { state.paramName, element, baseType };
    if (assignPointersToNull) {
        genConstraintsForPointerInStruct(newState);
    } else if (typesHandler->isStructLike(baseType)) {
        genConstraintsForStruct(newState);
    } else if (typesHandler->isEnum(baseType)) {
        genConstraintsForEnum(newState);
    } else {
        newState = { state.paramName, element, baseType };
        genConstraintsForPrimitive(newState);
    }

    closeBrackets(sizes.size());
}

void KleeConstraintsPrinter::genConstraintsForPointerInStruct(const ConstraintsState &state) {
    strFunctionCall(PrinterUtils::KLEE_ASSUME, { state.curElement + PrinterUtils::EQ_OPERATOR + PrinterUtils::C_NULL });
}

void KleeConstraintsPrinter::genConstraintsForStruct(const ConstraintsState &state) {
    StructInfo curStruct = typesHandler->getStructInfo(state.curType);
    bool isStruct = curStruct.subType == SubType::Struct;
    for (const auto &field : curStruct.fields) {
        auto access = PrinterUtils::getFieldAccess(state.curElement, field);
        ConstraintsState newState = { state.paramName,
                                      access,
                                      field.type,
                                      isStruct ? state.endString : false,
                                      state.depth + 1 };
        std::string stubFunctionName = StubsUtils::getFunctionPointerAsStructFieldStubName(curStruct.name, field.name);
        switch (typesHandler->getTypeKind(field.type)) {
        case TypeKind::STRUCT_LIKE:
            genConstraintsForStruct(newState);
            break;
        case TypeKind::ENUM:
            genConstraintsForEnum(newState);
            break;
        case TypeKind::PRIMITIVE:
            genConstraintsForPrimitive(newState);
            break;
        case TypeKind::ARRAY:
            genConstraintsForPointerOrArray(newState);
            break;
        case TypeKind::OBJECT_POINTER:
            if (types::TypesHandler::isArrayOfPointersToFunction(field.type)) {
                genStubForStructFunctionPointerArray(state.curElement, field,
                                                     stubFunctionName);
            }
            break;
        case TypeKind::FUNCTION_POINTER:
            genStubForStructFunctionPointer(state.curElement,
                                            field,
                                            stubFunctionName);
            break;
        case TypeKind::UNKNOWN: {
            std::string errorMessage = "Unrecognized field of type '" + field.type.typeName() +
                                       "' in struct '" + curStruct.name + "'.";
            throw UnImplementedException(errorMessage);
        }
        default:
            std::string message = "Missing case for this TypeKind in switch";
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
    }
}

std::string KleeConstraintsPrinter::cexConstraints(const std::string &name, const types::Type &type, const std::vector<std::string>& names) {
    if (!CollectionUtils::containsKey(TypesHandler::preferredConstraints(), type.baseType())) {
        return "";
    }
    std::stringstream ssCex;
    const std::vector<std::string> &constraints = TypesHandler::preferredConstraints().at(type.baseType());
    for (size_t i = 0; i < constraints.size(); i++) {
        ssCex << name << " " << constraints[i];
        if (i + 1 < constraints.size()) {
            ssCex << " & ";
        }
    }
    for (const std::string& currentName: names){
        if(name != currentName){
            ssCex << " & " << name << " != " << currentName;
        }
    }
    return ssCex.str();
}

utbot::Language printer::KleeConstraintsPrinter::getLanguage() const {
    return srcLanguage;
}
