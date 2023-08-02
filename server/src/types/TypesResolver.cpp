#include "TypesResolver.h"

#include "Paths.h"
#include "clang-utils/ASTPrinter.h"
#include "clang-utils/ClangUtils.h"
#include "fetchers/Fetcher.h"
#include "fetchers/FetcherUtils.h"
#include "utils/LogUtils.h"
#include "utils/path/FileSystemPath.h"

#include <clang/AST/ASTContext.h>

#include "loguru.h"

#include <vector>

TypesResolver::TypesResolver(const Fetcher *parent) : parent(parent) {
}

static bool canBeReplaced(const std::string &nameInMap, const std::string &name) {
    return nameInMap.empty() && !name.empty();
}

template<class Info>
bool isCandidateToReplace(uint64_t id,
                          std::unordered_map<uint64_t, Info> &someMap,
                          std::string const &name) {
    auto iterator = someMap.find(id);
    if (iterator != someMap.end()) {
        std::string nameInMap = iterator->second.name;
        return canBeReplaced(nameInMap, name);
    }
    return true;
}

static size_t getRecordSize(const clang::RecordDecl *D) {
    return D->getASTContext().getTypeSize(D->getASTContext().getRecordType(D));
}

static size_t getDeclAlignment(const clang::TagDecl *T) {
    return T->getASTContext().getTypeAlign(T->getTypeForDecl()) / 8;
}

template<class Info>
static void addInfo(uint64_t id, std::unordered_map<uint64_t, Info> &someMap, Info info) {
    auto [iterator, inserted] = someMap.emplace(id, info);
    LOG_IF_S(MAX, !inserted) << "Type with id=" << id << " already existed";
    if (!inserted) {
        std::string nameInMap = iterator->second.name;
        if (canBeReplaced(nameInMap, info.name)) {
            iterator->second = info;
            LOG_S(DEBUG) << "Replace unnamed type with typedef: " << info.name;
        } else if (!nameInMap.empty() && info.name.empty()) {
            LOG_S(MAX) << "Already replaced with typedef: " << nameInMap;
        } else if (nameInMap != info.name) {
            LOG_S(WARNING) << "Collision happened between: '" << nameInMap << "' and '" << info.name
                           << "'";
        }
    }
}

std::string TypesResolver::getFullname(const clang::TagDecl *TD, const clang::QualType &canonicalType,
                                       uint64_t id, const fs::path &sourceFilePath) {
    auto pp = clang::PrintingPolicy(clang::LangOptions());
    pp.SuppressTagKeyword = true;
    bool typeDeclNeeded = canonicalType->hasUnnamedOrLocalType() && !fullname[id].empty();
    std::string currentStructName = canonicalType.getNonReferenceType().getUnqualifiedType().getAsString(pp);
    fullname.insert(std::make_pair(id, currentStructName));

    if (Paths::getSourceLanguage(sourceFilePath) == utbot::Language::C || typeDeclNeeded) {
        if (const auto *parentNode = llvm::dyn_cast<const clang::RecordDecl>(TD->getLexicalParent())) {
            clang::QualType parentCanonicalType = parentNode->getASTContext().getTypeDeclType(
                    parentNode).getCanonicalType();
            uint64_t parentID = types::Type::getIdFromCanonicalType(parentCanonicalType);
            if (!fullname[parentID].empty()) {
                fullname[id] = fullname[parentID] + "::" + fullname[id];
                if (typeDeclNeeded) {
                    StringUtils::flatten(fullname[id]);
                }
            }
        }
    }
    return fullname[id];
}

void TypesResolver::resolveUnion(const clang::RecordDecl *D, const std::string &name) {
    resolveStructEx(D, name, types::SubType::Union);
}

void TypesResolver::resolveStruct(const clang::RecordDecl *D, const std::string &name) {
    resolveStructEx(D, name, types::SubType::Struct);
}

void TypesResolver::resolveStructEx(const clang::RecordDecl *D, const std::string &name, types::SubType subType) {
    clang::ASTContext const &context = D->getASTContext();
    clang::SourceManager const &sourceManager = context.getSourceManager();

    clang::QualType canonicalType = context.getTypeDeclType(D).getCanonicalType();
    uint64_t id = types::Type::getIdFromCanonicalType(canonicalType);
    if (!isCandidateToReplace(id, parent->projectTypes->structs, name)) {
        return;
    }

    types::StructInfo structInfo;
    fs::path filename =
            sourceManager.getFilename(sourceManager.getSpellingLoc(D->getLocation())).str();
    fs::path sourceFilePath = ClangUtils::getSourceFilePath(sourceManager);
    structInfo.filePath = Paths::getCCJsonFileFullPath(filename, parent->buildRootPath);
    structInfo.name = getFullname(D, canonicalType, id, sourceFilePath);
    structInfo.hasAnonymousStructOrUnion = false;
    if (Paths::getSourceLanguage(sourceFilePath) == utbot::Language::CXX) {
        const auto *cppD = llvm::dyn_cast<const clang::CXXRecordDecl>(D);
        structInfo.isCLike = cppD != nullptr && cppD->isCLike();
    } else {
        structInfo.isCLike = true;
    }

    if (Paths::isGtest(structInfo.filePath)) {
        return;
    }

    std::stringstream ss;
    structInfo.definition = ASTPrinter::getSourceText(D->getSourceRange(), sourceManager);
    ss << "Struct: " << structInfo.name << "\n"
       << "\tFile path: " << structInfo.filePath.string() << "";
    std::vector<types::Field> fields;

    for (const clang::FieldDecl *F: D->fields()) {
        if (F->isUnnamedBitfield()) {
            continue;
        }
        types::Field field;
        field.anonymous = F->isAnonymousStructOrUnion();
        field.name = F->getNameAsString();
        structInfo.hasAnonymousStructOrUnion |= field.anonymous;

        const clang::QualType paramType = F->getType().getCanonicalType();
        field.type = types::Type(paramType, paramType.getAsString(), sourceManager);
        field.unnamedType = field.type.isUnnamed();
        if (field.unnamedType && !field.anonymous) {
            fullname[field.type.getId()] = field.name;
        }
        if (field.type.isPointerToFunction()) {
            structInfo.functionFields[field.name] = ParamsHandler::getFunctionPointerDeclaration(
                    F->getFunctionType(), field.name, sourceManager,
                    field.type.isArrayOfPointersToFunction());
            auto returnType = F->getFunctionType()->getReturnType();
            if (returnType->isPointerType()
                && returnType->getPointeeType()->isStructureType()
                && returnType->getPointeeType().getBaseTypeIdentifier()) {
                std::string structName =
                        returnType->getPointeeType().getBaseTypeIdentifier()->getName().str();
                if (!CollectionUtils::containsKey((*parent->structsDeclared).at(sourceFilePath),
                                                  structName)) {
                    (*parent->structsToDeclare)[sourceFilePath].insert(structName);
                }
            }
        } else if (field.type.isArrayOfPointersToFunction()) {
            structInfo.functionFields[field.name] = ParamsHandler::getFunctionPointerDeclaration(
                    F->getType()->getPointeeType()->getPointeeType()->getAs<clang::FunctionType>(),
                    field.name, sourceManager, field.type.isArrayOfPointersToFunction());
        }
        field.size = F->isBitField() ? F->getBitWidthValue(context) : context.getTypeSize(F->getType());
        field.offset = context.getFieldOffset(F);
        if (LogUtils::isMaxVerbosity()) {
            ss << "\n\t" << field.type.typeName() << " " << field.name << ";";
        }
        if (Paths::getSourceLanguage(sourceFilePath) == utbot::Language::CXX) {
            field.accessSpecifier = getAcessSpecifier(F);
        } else {
            field.accessSpecifier = types::AccessSpecifier::AS_pubic;
        }
        fields.push_back(field);
    }
    structInfo.fields = fields;
    structInfo.size = getRecordSize(D);
    structInfo.alignment = getDeclAlignment(D);
    structInfo.subType = subType;
    structInfo.hasDefaultPublicConstructor = false;
    if (auto CXXD = dynamic_cast<const clang::CXXRecordDecl *>(D)) {
        LOG_S(MAX) << "Struct/Class " << structInfo.name << " CXX class";
        if (!CXXD->isCLike()) {
            structInfo.isCLike = false;
        }
        for (const auto &ctor: CXXD->ctors()) {
            if (ctor->isDefaultConstructor() && !ctor->isDeleted() &&
                    getAcessSpecifier(ctor) == types::AccessSpecifier::AS_pubic) {
                structInfo.hasDefaultPublicConstructor = true;
                break;
            }
        }
        LOG_IF_S(MAX, !structInfo.hasDefaultPublicConstructor) << "Struct/Class "
                                                               << structInfo.name
                                                               << " hasn't default public constructor";
    }

    addInfo(id, parent->projectTypes->structs, structInfo);
    ss << "\nName: " << structInfo.name << ", id: " << id << " , size: " << structInfo.size << "\n";

    updateMaximumAlignment(structInfo.alignment);

    LOG_S(DEBUG) << ss.str();
}

bool isSpecifierNeeded(const clang::TagDecl *tagDecl) {
    return tagDecl->getTypedefNameForAnonDecl() == nullptr;
}

static std::optional<std::string> getAccess(const clang::Decl *decl) {
    const clang::DeclContext *pContext = decl->getDeclContext();
    std::vector<std::string> result;
    while (pContext != nullptr) {
        if (auto pNamedDecl = llvm::dyn_cast<clang::NamedDecl>(pContext)) {
            auto name = pNamedDecl->getNameAsString();
            if (!name.empty()) {
                result.push_back(name);
            }
            pContext = pContext->getParent();
        } else {
            break;
        }
    }
    if (result.empty()) {
        return std::nullopt;
    }
    return StringUtils::joinWith(llvm::reverse(result), "::");
}

void TypesResolver::resolveEnum(const clang::EnumDecl *EN, const std::string &name) {
    clang::ASTContext const &context = EN->getASTContext();
    clang::SourceManager const &sourceManager = context.getSourceManager();

    clang::QualType canonicalType = context.getTypeDeclType(EN).getCanonicalType();
    uint64_t id = types::Type::getIdFromCanonicalType(canonicalType);
    if (!isCandidateToReplace(id, parent->projectTypes->enums, name)) {
        return;
    }

    types::EnumInfo enumInfo;
    fs::path sourceFilePath = ClangUtils::getSourceFilePath(sourceManager);
    enumInfo.name = getFullname(EN, canonicalType, id, sourceFilePath);
    enumInfo.filePath = Paths::getCCJsonFileFullPath(
            sourceManager.getFilename(EN->getLocation()).str(), parent->buildRootPath.string());
    clang::QualType promotionType = EN->getPromotionType();
    enumInfo.size = context.getTypeSize(promotionType);

    enumInfo.access = getAccess(EN);
    enumInfo.isSpecifierNeeded = isSpecifierNeeded(EN);

    for (auto it = EN->enumerator_begin(); it != EN->enumerator_end(); ++it) {
        types::EnumInfo::EnumEntry enumEntry;
        std::string entryName = it->getNameAsString();
        enumEntry.name = entryName;
        enumEntry.value = std::to_string(it->getInitVal().getSExtValue());
        enumInfo.valuesToEntries[enumEntry.value] = enumEntry;
        enumInfo.namesToEntries[enumEntry.name] = enumEntry;
    }
    enumInfo.definition = ASTPrinter::getSourceText(EN->getSourceRange(), sourceManager);
    enumInfo.alignment = getDeclAlignment(EN);

    addInfo(id, parent->projectTypes->enums, enumInfo);
    LOG_S(DEBUG) << "\nName: " << enumInfo.name << ", id: " << id << "\n";

    updateMaximumAlignment(enumInfo.alignment);

    std::stringstream ss;
    ss << "EnumInfo: " << enumInfo.name << "\n"
       << "\tFile path: " << enumInfo.filePath.string();
    LOG_S(DEBUG) << ss.str();
}

void TypesResolver::updateMaximumAlignment(size_t alignment) const {
    size_t &maximumAlignment = *(this->parent->maximumAlignment);
    maximumAlignment = std::max(maximumAlignment, alignment);
}


void TypesResolver::resolve(const clang::QualType &type) {
    clang::TagDecl *tagDecl = type->getAsTagDecl();
    if (tagDecl == nullptr) {
        return;
    }
    std::string name = tagDecl->getNameAsString();
    if (auto enumDecl = llvm::dyn_cast<clang::EnumDecl>(tagDecl)) {
        resolveEnum(enumDecl, name);
    }
    if (auto recordDecl = llvm::dyn_cast<clang::RecordDecl>(tagDecl)) {
        if (recordDecl->isStruct() || recordDecl->isClass()) {
            resolveStruct(recordDecl, name);
        }
        if (recordDecl->isUnion()) {
            resolveUnion(recordDecl, name);
        }
    }
}
