#include "TypeVisitor.h"

#include "ArrayType.h"
#include "FunctionPointerType.h"
#include "ObjectPointerType.h"
#include "SimpleType.h"
#include "Types.h"

#include <llvm/Support/Casting.h>

bool TypeVisitor::TraverseType(clang::QualType type) {
    clang::QualType canonicalType = type.getCanonicalType();
    auto pp = clang::PrintingPolicy(clang::LangOptions());
    pp.adjustForCPlusPlus();
    const auto curType = canonicalType.getNonReferenceType().getUnqualifiedType();
    const auto curTypeString = curType.getAsString();
    const auto curTypeStringForCPlusPlus = curType.getAsString(pp);
    if (types.empty() || curTypeString != types.back()) {
        types.push_back(curTypeString);
        typesForCPlusPlus.push_back(curTypeStringForCPlusPlus);
        if (curType->isFunctionPointerType()) {
            kinds.push_back(std::make_shared<FunctionPointerType>());
        } else if (curType->isObjectPointerType()) {
            bool constQualified = canonicalType.isConstQualified();
            kinds.push_back(std::make_shared<ObjectPointerType>(constQualified));
        } else if (curType->isConstantArrayType()) {
            const auto constArray = llvm::dyn_cast<clang::ConstantArrayType>(curType.getTypePtr());
            uint64_t size = constArray->getSize().getZExtValue();
            kinds.push_back(std::make_shared<ArrayType>(size, true));
        } else if (curType->isIncompleteArrayType()) {
            uint64_t size = 0;
            kinds.push_back(std::make_shared<ArrayType>(size, false));
        } else {
            uint64_t id = ::types::Type::getIdFromCanonicalType(curType);
            bool unnamed = StringUtils::contains(curTypeString, "anonymous at");
            bool constQualified = canonicalType.getNonReferenceType().isConstQualified();
            SimpleType::ReferenceType referenceType = SimpleType::ReferenceType::NotReference;
            if (canonicalType->isLValueReferenceType()) {
                referenceType = SimpleType::ReferenceType::LValueReference;
            }
            if (canonicalType->isRValueReferenceType()) {
                referenceType = SimpleType::ReferenceType::RValueReference;
            }
            bool rValue = canonicalType->isRValueReferenceType();
            kinds.push_back(std::make_shared<SimpleType>(id, unnamed, constQualified, referenceType));
        }
    }
    RecursiveASTVisitor<TypeVisitor>::TraverseType(curType);
    return true;
}

std::vector<std::shared_ptr<AbstractType>> TypeVisitor::getKinds() {
    return kinds;
}

std::vector<std::string> TypeVisitor::getTypes() {
    return types;
}

std::vector<std::string> TypeVisitor::getTypesForCPlusPlus() {
    return typesForCPlusPlus;
}
