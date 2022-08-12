#include "KleeConstraintsPrinter.h"

#include "utils/PrinterUtils.h"
#include "exceptions/UnImplementedException.h"

#include "loguru.h"

#include <cstddef>

using namespace types;
using printer::KleeConstraintsPrinter;

printer::KleeConstraintsPrinter::KleeConstraintsPrinter(const types::TypesHandler *typesHandler, utbot::Language srcLanguage)
    : Printer(srcLanguage), typesHandler(typesHandler) {}

printer::KleeConstraintsPrinter::Stream
KleeConstraintsPrinter::genConstraints(const std::string &name, const types::Type& type) {
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
        case TypeKind::STRUCT:
            genConstraintsForStruct(state);
            break;
        case TypeKind::ENUM:
            genConstraintsForEnum(state);
            break;
        case TypeKind::UNION:
            genConstraintsForUnion(state);
            break;
        default:
            genConstraintsForPrimitive(state);
    }

    return ss;
}

printer::KleeConstraintsPrinter::Stream
KleeConstraintsPrinter::genConstraints(const Tests::MethodParam &param) {
    return genConstraints(param.name, param.type);
}

void KleeConstraintsPrinter::genConstraintsForPrimitive(const ConstraintsState &state) {
    const auto &cons = cexConstraints(state.curElement, state.curType);
    if (!cons.empty()) {
        strFunctionCall(PrinterUtils::KLEE_PREFER_CEX, { state.paramName, cons });
    } else {
        ss << TAB_N() << "// No constraints for " << state.curElement << NL;
    }
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

void KleeConstraintsPrinter::genConstraintsForUnion(const ConstraintsState &state) {
    UnionInfo curUnion = typesHandler->getUnionInfo(state.curType);

    for (const auto &field : curUnion.fields) {
        std::string errorMessage = "Unrecognized field of type '" + field.type.typeName() +
                                   "' in union '" + curUnion.name + "'.";
        auto access = PrinterUtils::getFieldAccess(state.curElement, field);
        ConstraintsState newState = { state.paramName, access, field.type, false, state.depth + 1 };
        switch (typesHandler->getTypeKind(field.type)) {
        case TypeKind::PRIMITIVE:
            return genConstraintsForPrimitive(newState);
        case TypeKind::STRUCT:
            return genConstraintsForStruct(newState);
        case TypeKind::ARRAY:
            return genConstraintsForPointerOrArray(newState);
        case TypeKind::ENUM:
            return genConstraintsForEnum(newState);
        case TypeKind::UNION:
            return genConstraintsForUnion(newState);
        case TypeKind::OBJECT_POINTER:
            return genConstraintsForPointerInUnion(newState);
        case TypeKind::UNKNOWN:
            LOG_S(ERROR) << errorMessage;
            throw UnImplementedException(errorMessage);
        default:
            std::string message = "Missing case for this TypeKind in switch";
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
    }
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
        ss << TAB_N() << "if (" << indexes.back() << PrinterUtils::EQ_OPERATOR << sizes.back() - 1 << ")" << LB();
        ss << TAB_N() << PrinterUtils::KLEE_ASSUME << "(" << charElement << "[" << sizes.back() - 1 << "]" << PrinterUtils::EQ_OPERATOR << "'\\0'" << ")" << SCNL;
        ss << TAB_N() << "break" << SCNL;
        ss << RB();
    }

    ConstraintsState newState = { state.paramName, element, baseType };
    if (assignPointersToNull) {
        genConstraintsForPointerInStruct(newState);
    } else if (typesHandler->isStruct(baseType)) {
        genConstraintsForStruct(newState);
    } else if (typesHandler->isEnum(baseType)) {
        genConstraintsForEnum(newState);
    } else if (typesHandler->isUnion(baseType)) {
        genConstraintsForUnion(newState);
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

    for (const auto &field : curStruct.fields) {
        std::string errorMessage = "Unrecognized field of type '" + field.type.typeName() +
                                   "' in struct '" + curStruct.name + "'.";
        auto access = PrinterUtils::getFieldAccess(state.curElement, field);
        ConstraintsState newState = { state.paramName, access, field.type, state.endString, state.depth + 1 };
        TypeKind kind = typesHandler->getTypeKind(field.type);
        std::string stubFunctionName = PrinterUtils::getFunctionPointerAsStructFieldStubName(curStruct.name, field.name);
        switch (kind) {
        case TypeKind::PRIMITIVE:
            genConstraintsForPrimitive(newState);
            break;
        case TypeKind::STRUCT:
            genConstraintsForStruct(newState);
            break;
        case TypeKind::ARRAY:
            genConstraintsForPointerOrArray(newState);
            break;
        case TypeKind::ENUM:
            genConstraintsForEnum(newState);
            break;
        case TypeKind::UNION:
            genConstraintsForUnion(newState);
            break;
        case TypeKind::OBJECT_POINTER:
            if (types::TypesHandler::isArrayOfPointersToFunction(field.type)) {
                genStubForStructFunctionPointerArray(state.curElement, field.name, stubFunctionName);
            }
            break;
        case TypeKind::FUNCTION_POINTER:
            genStubForStructFunctionPointer(state.curElement, field.name, stubFunctionName);
            break;
        case TypeKind::UNKNOWN:
            throw UnImplementedException(errorMessage);
        default:
            std::string message = "Missing case for this TypeKind in switch";
            LOG_S(ERROR) << message;
            throw NoSuchTypeException(message);
        }
    }
}

std::string KleeConstraintsPrinter::cexConstraints(const std::string &name, const types::Type &type) {
    if (!CollectionUtils::containsKey(TypesHandler::preferredConstraints(), type.baseType())) {
        return "";
    }
    std::stringstream ssCex;
    const std::vector<std::string> &constraints = TypesHandler::preferredConstraints().at(type.baseType());
    for (size_t i = 0; i < constraints.size(); i++) {
        ssCex << name << " " << constraints[i];
        if (i + 1 < constraints.size()) {
            ssCex << " && ";
        }
    }
    return ssCex.str();
}

void printer::KleeConstraintsPrinter::genConstraintsForPointerInUnion(
    const ConstraintsState &state) {
    strFunctionCall(PrinterUtils::KLEE_ASSUME, { state.curElement + PrinterUtils::EQ_OPERATOR + PrinterUtils::C_NULL });
}

utbot::Language printer::KleeConstraintsPrinter::getLanguage() const {
    return srcLanguage;
}
