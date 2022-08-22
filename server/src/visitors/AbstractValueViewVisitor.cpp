#include "AbstractValueViewVisitor.h"

namespace visitor {
    AbstractValueViewVisitor::AbstractValueViewVisitor(const types::TypesHandler *typesHandler,
                                                       types::PointerUsage usage)
        : typesHandler(typesHandler), usage(usage) {
    }

    void AbstractValueViewVisitor::visitAny(const types::Type &type,
                                            const std::string &name,
                                            const tests::AbstractValueView *view,
                                            const std::string &access,
                                            int depth) {
        if (types::TypesHandler::isCStringType(type)) {
            return visitCString(type, name, view, access, depth);
        }
        if (type.isArray() || type.isPointerToArray() || type.isPointerToPointer() && depth == 0) {
            size_t size = type.kinds().front()->getSize();
            if (types::TypesHandler::isVoid(type.baseTypeObj())) {
                return visitArray(
                    types::Type::minimalScalarPointerType(type.arraysSizes(usage).size()), name,
                    view, access, size, depth);
            } else {
                return visitArray(type, name, view, access, size, depth);
            }
        } else if (types::TypesHandler::isArrayOfPointersToFunction(type)) {
            return visitPointerToFunction(type, name, view, access, depth);
        } else if (types::TypesHandler::isObjectPointerType(type)) {
            if (types::TypesHandler::isVoid(type.baseTypeObj())) {
                return visitPointer(types::Type::minimalScalarPointerType(), name, view, access, depth);
            } else {
                return visitPointer(type, name, view, access, depth);
            }
        } else if (typesHandler->isStruct(type)) {
            return visitStruct(type, name, view, access, depth);
        } else if (typesHandler->isEnum(type)) {
            return visitEnum(type, name, view, access, depth);
        } else if (types::TypesHandler::isPointerToFunction(type)) {
            return visitPointerToFunction(type, name, view, access, depth);
        } else if (types::TypesHandler::isPrimitiveType(type)) {
            return visitPrimitive(type, name, view, access, depth);
        }
    }

    void AbstractValueViewVisitor::visitArrayElementAfter(const types::Type &type,
                                                          const std::string &name,
                                                          const tests::AbstractValueView *view,
                                                          const std::string &access,
                                                          int depth) {
    }

    void AbstractValueViewVisitor::visitPointer(const types::Type &type,
                                                const std::string &name,
                                                const tests::AbstractValueView *view,
                                                const std::string &access,
                                                int depth) {
        size_t size =
            types::TypesHandler::getElementsNumberInPointerOneDim(usage);
        visitArray(type, name, view, access, size, depth);
    }

    void AbstractValueViewVisitor::visitArray(const types::Type &type,
                                              const std::string &name,
                                              const tests::AbstractValueView *view,
                                              const std::string &access,
                                              size_t size,
                                              int depth) {
        auto subViews = view ? &view->getSubViews() : nullptr;
        for (int i = 0; i < size; i++) {
            auto index = "[" + std::to_string(i) + "]";
            types::Type newType = type.baseTypeObj(1);
            auto const newName = name + index;
            auto const *newView = subViews ? (*subViews)[i].get() : nullptr;
            auto const newAccess = access + index;
            visitAny(newType, newName, newView, newAccess, depth + 1);
            visitArrayElementAfter(newType, newName, newView, newAccess, depth + 1);
        }
    }
    void AbstractValueViewVisitor::visitCString(const types::Type &type,
                                                const std::string &name,
                                                const tests::AbstractValueView *view,
                                                const std::string &access,
                                                int depth) {
        visitPointer(type, name, view, access, depth);
    }

    void AbstractValueViewVisitor::visitStruct(const types::Type &type,
                                              const std::string &name,
                                              const tests::AbstractValueView *view,
                                              const std::string &access,
                                              int depth) {
        const types::StructInfo &structInfo = typesHandler->getStructInfo(type);
        auto subViews = view ? &view->getSubViews() : nullptr;

        bool oldFlag = inUnion;
        inUnion = structInfo.subType == types::SubType::Union;
        for (int i = 0; i < structInfo.fields.size(); ++i) {
            auto const &field = structInfo.fields[i];
            auto newName = PrinterUtils::getFieldAccess(name, field);
            auto const *newView = (subViews && i < subViews->size()) ? (*subViews)[i].get() : nullptr;
            auto newAccess = PrinterUtils::getFieldAccess(access, field);
            visitAny(field.type, newName, newView, newAccess, depth + 1);
        }
        inUnion = oldFlag;
    }
    void AbstractValueViewVisitor::visitEnum(const types::Type &type,
                                             const std::string &name,
                                             const tests::AbstractValueView *view,
                                             const std::string &access,
                                             int depth) {
        visitPrimitive(type, name, view, access, depth);
    }

    void AbstractValueViewVisitor::visitPointerToFunction(const types::Type &type,
                                                          const std::string &name,
                                                          const tests::AbstractValueView *view,
                                                          const std::string &access,
                                                          int depth) {
    }

    std::string AbstractValueViewVisitor::getDecoratedVarName(const std::string &varName,
                                                              size_t pointersCount,
                                                              const std::string &access) {
        if (access.empty() || pointersCount == 0) {
            return StringUtils::repeat("*", pointersCount) +
                    PrinterUtils::fillVarName(access, varName);
        } else {
            return PrinterUtils::fillVarName(access, "(" + StringUtils::repeat("*", pointersCount) +
                                                     varName + ")");
        }
    }
}
