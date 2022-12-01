#include "Types.h"

#include "ArrayType.h"
#include "ObjectPointerType.h"
#include "SimpleType.h"
#include "TypeVisitor.h"
#include "exceptions/UnImplementedException.h"
#include "utils/PrinterUtils.h"
#include "utils/SizeUtils.h"

#include "loguru.h"

#include <climits>

/*
 * class Type
 */
types::Type::Type(clang::QualType qualType, TypeName usedTypeName, const clang::SourceManager &sourceManager): mUsedType(std::move(usedTypeName)) {
    clang::QualType canonicalType = qualType.getCanonicalType();
    auto pp = clang::PrintingPolicy(clang::LangOptions());
    fs::path sourceFilePath = sourceManager.getFileEntryForID(sourceManager.getMainFileID())->tryGetRealPathName().str();
    if (Paths::getSourceLanguage(sourceFilePath) == utbot::Language::CXX) {
        pp.adjustForCPlusPlus();
    }
    mType = canonicalType.getNonReferenceType().getUnqualifiedType().getAsString(pp);
    TypeVisitor visitor;
    visitor.TraverseType(qualType);
    mKinds = visitor.getKinds();
    dimension = getDimension();
    mBaseType = visitor.getTypes()[dimension];
    mTypeId = getIdFromCanonicalType(canonicalType);
    AbstractType *baseType = mKinds[dimension].get();
    if (auto simpleType = dynamic_cast<SimpleType*>(baseType)) {
        mBaseTypeId = simpleType->getId();
    }
}

uint64_t types::Type::getIdFromCanonicalType(clang::QualType canonicalType) {
    auto unqualifiedType = canonicalType.getNonReferenceType().getUnqualifiedType();
    if (auto tagType = llvm::dyn_cast<clang::TagType>(unqualifiedType)) {
        auto decl = tagType->getDecl();
        clang::SourceManager const &sourceManager = decl->getASTContext().getSourceManager();
        const clang::SourceLocation spellingLoc = sourceManager.getSpellingLoc(decl->getLocation());
        fs::path filePath = sourceManager.getFilename(spellingLoc).str();
        std::string name = unqualifiedType.getAsString();
        uint64_t id = 0;
        HashUtils::hashCombine(id, name, filePath);
        LOG_S(DEBUG) << "Name: " << name << ", file: " << filePath <<  ", id: " << id;
        return id;
    } else {
        return 0;
    }
}

types::Type::Type(const types::TypeName& type, size_t pointersNum) {
    if (pointersNum > 0) {
        mType = type + " " + std::string(pointersNum, '*');
    } else {
        mType = type;
    }
    mUsedType = mType;
    dimension = pointersNum;
    mBaseType = type;
    for (size_t i = 0; i < pointersNum; ++i) {
        mKinds.push_back(std::make_shared<ObjectPointerType>(false));
    }
    mKinds.push_back(std::make_shared<SimpleType>(0, false, false, SimpleType::ReferenceType::NotReference));
}

types::Type types::Type::createSimpleTypeFromName(const types::TypeName& type, size_t pointersNum) {
    return Type(type, pointersNum);
}

types::Type types::Type::createConstTypeFromName(const types::TypeName& type, size_t pointersNum) {
    auto res = createSimpleTypeFromName(type, pointersNum);
    res.mType = "const " + res.mType;
    res.mUsedType = res.mType;
    return res;
}

types::Type types::Type::createArray(const types::Type &type) {
    Type res;
    res.mType = type.typeName() + "*";
    res.mUsedType = res.mType;
    res.mBaseType = type.baseType();
    res.mKinds = type.mKinds;
    res.mKinds.insert(res.mKinds.begin(), std::shared_ptr<AbstractType>(new ArrayType(
        TypesHandler::getElementsNumberInPointerOneDim(PointerUsage::PARAMETER), false)));
    res.dimension = type.dimension + 1;
    res.mTypeId = 0;
    res.mBaseTypeId = type.mBaseTypeId;
    res.maybeArray = true;
    return res;
}

types::TypeName types::Type::typeName() const {
    return mType;
}

types::TypeName types::Type::baseType() const {
    return mBaseType;
}

types::TypeName types::Type::usedType() const {
    return mUsedType;
}

types::Type types::Type::baseTypeObj(size_t depth) const {
    auto type = *this;
    type.mType = mBaseType;
    type.mBaseType = type.mType;
    type.mUsedType = type.mType;
    type.mKinds.erase(type.mKinds.begin(), type.mKinds.begin() + depth);
    type.dimension = type.getDimension();
    type.mTypeId = mBaseTypeId;
    type.mBaseTypeId = {};
    return type;
}

types::Type types::Type::baseTypeObj() const {
    return baseTypeObj(getDimension());
}

std::string types::Type::mTypeName() const {
    return this->mType;
}

size_t types::Type::getDimension() const {
    // usage doesn't matter here
    return arraysSizes(PointerUsage::PARAMETER).size();
}

std::optional<uint64_t> types::Type::getBaseTypeId() const {
    return mBaseTypeId;
}

bool types::Type::maybeJustPointer() const {
    return types::TypesHandler::isObjectPointerType(*this) && !maybeArray &&
        this->kinds().size() < 3;
}

bool types::Type::isFilePointer() const {
    return typeName() == FILE_PTR_TYPE;
}

bool types::Type::maybeReturnArray() const {
    bool condition1 = types::TypesHandler::isObjectPointerType(*this);
    bool condition2 = maybeJustPointer();
    bool condition3 = types::TypesHandler::isArrayOfPointersToFunction(*this);
    bool condition4 = this->kinds().size() < 3;
    bool condition5 = types::TypesHandler::isVoid(this->baseTypeObj());
    return condition1 && !condition2 && !condition3 && condition4 && !condition5;
}

size_t types::Type::countReturnPointers(bool decrementIfArray) const {
    size_t returnPointer = 0;
    const std::vector<std::shared_ptr<AbstractType>> pointerArrayKinds = this->pointerArrayKinds();
    for (const auto &pointerArrayKind: pointerArrayKinds) {
        returnPointer += pointerArrayKind->getKind() == AbstractType::OBJECT_POINTER;
    }
    if (decrementIfArray && maybeReturnArray()) {
        returnPointer--;
    }
    return returnPointer;
}

const std::vector<std::shared_ptr<AbstractType>> &types::Type::kinds() const {
    return mKinds;
}

std::vector<size_t> types::Type::arraysSizes(PointerUsage usage) const {
    if (!isArray() && !isObjectPointer() && !isPointerToFunction()) {
        return {};
    }
    std::vector<size_t> sizes;
    for (const auto &kind: pointerArrayKinds()) {
        switch (kind->getKind()) {
        case AbstractType::ARRAY:
            sizes.push_back(kind->getSize());
            break;
        case AbstractType::OBJECT_POINTER:
        case AbstractType::FUNCTION_POINTER:
            if (kinds().size() <= 2) {
                sizes.push_back(types::TypesHandler::getElementsNumberInPointerOneDim(usage));
            } else {
                sizes.push_back(types::TypesHandler::getElementsNumberInPointerMultiDim(usage));
            }
            if (usage == types::PointerUsage::LAZY) {
                return sizes;
            }
            break;
        default:
            LOG_S(ERROR) << "INVARIANT ERROR: Class Type: " << kind->getKind();
        }
    }
    return sizes;
}

std::vector<std::shared_ptr<AbstractType>> types::Type::pointerArrayKinds() const {
    if (kinds().size() <= 1) {
        return {};
    }

    std::vector<std::shared_ptr<AbstractType>> res;
    res.reserve(kinds().size() - 1);

    size_t i = 0;
    while (i < kinds().size() - 1 &&
        (mKinds[i]->getKind() == AbstractType::OBJECT_POINTER || mKinds[i]->getKind() == AbstractType::ARRAY)) {
        res.push_back(mKinds[i]);
        ++i;
    }

    return res;
}

bool types::Type::isArrayCandidate() const {
    return isObjectPointer() || isArray();
}

bool types::Type::isObjectPointer() const {
    return mKinds.front()->getKind() == AbstractType::OBJECT_POINTER;
}

bool types::Type::isArray() const {
    return mKinds.front()->getKind() == AbstractType::ARRAY;
}

bool types::Type::isPointerToFunction() const {
    return mKinds.front()->getKind() == AbstractType::FUNCTION_POINTER;
}

bool types::Type::isArrayOfPointersToFunction() const {
    return mKinds.size() > 1 &&
           mKinds[0]->getKind() == AbstractType::OBJECT_POINTER &&
           mKinds[1]->getKind() == AbstractType::FUNCTION_POINTER;
}

bool types::Type::isSimple() const {
    return mKinds.front()->getKind() == AbstractType::SIMPLE;
}

bool types::Type::isUnnamed() const {
    return isSimple() && dynamic_cast<SimpleType *>(mKinds.front().get())->isUnnamed();
}

bool types::Type::isLValueReference() const {
    return isSimple() && dynamic_cast<SimpleType *>(mKinds.front().get())->isLValue();
}

bool types::Type::isRValueReference() const {
    return isSimple() && dynamic_cast<SimpleType *>(mKinds.front().get())->isRValue();
}

bool types::Type::isConstQualified() const {
    return isSimple() && dynamic_cast<SimpleType *>(mKinds.front().get())->isConstQualified();
}

static const types::TypeName MINIMAL_SCALAR_TYPE = "unsigned char";

types::Type types::Type::minimalScalarType() {
    static types::Type minimalScalarTypeSingleton = createSimpleTypeFromName(MINIMAL_SCALAR_TYPE);
    return minimalScalarTypeSingleton;
}

types::Type types::Type::minimalScalarPointerType(size_t pointersNum) {
    return createSimpleTypeFromName(MINIMAL_SCALAR_TYPE, pointersNum);
}

types::Type types::Type::intType() {
    static types::Type intTypeSingleton = createSimpleTypeFromName("int");
    return intTypeSingleton;
}

types::Type types::Type::longlongType() {
    static types::Type longlongTypeSingleton = createSimpleTypeFromName("long long");
    return longlongTypeSingleton;
}

types::Type types::Type::CStringType() {
    static types::Type cStringTypeSingleton = createConstTypeFromName("char", 1);
    cStringTypeSingleton.maybeArray = true;
    return cStringTypeSingleton;
}

const std::string &types::Type::getStdinParamName() {
    static const std::string stdinParamName = "stdin_buf";
    return stdinParamName;
}

bool types::Type::isPointerToPointer() const {
    const std::vector<std::shared_ptr<AbstractType>> pointerArrayKinds = this->pointerArrayKinds();
    return pointerArrayKinds.size() > 1 &&
           pointerArrayKinds[0]->getKind() == AbstractType::OBJECT_POINTER &&
           pointerArrayKinds[1]->getKind() == AbstractType::OBJECT_POINTER;
}


bool types::Type::isTwoDimensionalPointer() const {
    return isPointerToPointer() && kinds().size() == 3;
}

bool types::Type::isPointerToArray() const {
    const std::vector<std::shared_ptr<AbstractType>> pointerArrayKinds = this->pointerArrayKinds();
    return pointerArrayKinds.size() > 1 &&
           pointerArrayKinds[0]->getKind() == AbstractType::OBJECT_POINTER &&
           pointerArrayKinds[1]->getKind() == AbstractType::ARRAY;
}

bool types::Type::isConstQualifiedValue() const {
    for (const auto &kind: mKinds) {
        if(kind->getKind() == AbstractType::SIMPLE) {
            if(dynamic_cast<SimpleType*>(kind.get())->isConstQualified()) {
                return true;
            } else {
                return false;
            }
        } else if (kind->getKind() != AbstractType::ARRAY &&
                   kind->getKind() != AbstractType::OBJECT_POINTER) {
            return false;
        }
    }
    return false;
}

bool types::Type::isTypeContainsPointer() const {
    const std::vector<std::shared_ptr<AbstractType>> pointerArrayKinds = this->pointerArrayKinds();
    return std::any_of(pointerArrayKinds.cbegin(), pointerArrayKinds.cend(),
                       [](auto const &kind){ return kind->getKind() == AbstractType::OBJECT_POINTER; });
}

bool types::Type::isTypeContainsFunctionPointer() const {
    return std::any_of(mKinds.cbegin(), mKinds.cend(),
                       [](auto const &kind){ return kind->getKind() == AbstractType::FUNCTION_POINTER; });
}

int types::Type::indexOfFirstPointerInTypeKinds() const {
    int index = 0;
    for (const auto &kind : pointerArrayKinds()) {
        if (kind->getKind() == AbstractType::OBJECT_POINTER) {
            return index;
        }
        index++;
    }
    throw BaseException("Couldn't find pointer in type " + typeName());
}

bool types::Type::isOneDimensionPointer() const {
    return isObjectPointer() && !isPointerToPointer() && !isPointerToArray();
}

types::Type types::Type::arrayClone(PointerUsage usage, size_t pointerSize) const {
    Type t = *this;
    t.mKinds[0] = std::make_shared<ArrayType>(TypesHandler::getElementsNumberInPointerOneDim(usage, pointerSize), true);
    return t;
}

types::Type types::Type::arrayCloneMultiDim(PointerUsage usage, std::vector<size_t> pointerSizes) const {
    Type t = *this;
    for(size_t i = 0; i < pointerSizes.size(); ++i) {
        if (t.mKinds[i]->getKind() == AbstractType::OBJECT_POINTER) {
            t.mKinds[i] = std::make_shared<ArrayType>(
                TypesHandler::getElementsNumberInPointerMultiDim(usage, pointerSizes[i]),
                true);
        }
    }
    return t;
}

types::Type types::Type::arrayCloneMultiDim(PointerUsage usage) const {
    if (this->maybeJustPointer() && this->pointerArrayKinds().size() < 2) {
        return this->baseTypeObj();
    }

    std::vector<size_t> pointerSizes = this->arraysSizes(usage);

    if(pointerSizes.size() == 1) {
        return arrayClone(usage);
    }

    return this->arrayCloneMultiDim(usage, pointerSizes);
}

uint64_t types::Type::getId() const {
    return mTypeId.value_or(0);
}

void types::Type::replaceUsedType(const types::TypeName &newUsedType) {
    mUsedType = newUsedType;
}

/*
 * Integer types
 */
static const std::unordered_map<std::string, size_t> integerTypesToSizes = {
        {"utbot_byte",         SizeUtils::bytesToBits(sizeof(char))}, // we use different name to not trigger char processing
        {"short",              SizeUtils::bytesToBits(sizeof(short))},
        {"int",                SizeUtils::bytesToBits(sizeof(int))},
        {"long",               SizeUtils::bytesToBits(sizeof(long))},
        {"long long",          SizeUtils::bytesToBits(sizeof(long long))},
        {"unsigned short",     SizeUtils::bytesToBits(sizeof(unsigned short))},
        {"unsigned int",       SizeUtils::bytesToBits(sizeof(unsigned int))},
        {"unsigned long",      SizeUtils::bytesToBits(sizeof(unsigned long))},
        {"unsigned long long", SizeUtils::bytesToBits(sizeof(unsigned long long))},
        {"unsigned char",      SizeUtils::bytesToBits(sizeof(unsigned char))} // we do not want to treat an unsigned char as character literal
};

bool types::TypesHandler::isIntegerType(const Type &type) {
    return type.isSimple() && isIntegerType(type.baseType());
}

bool types::TypesHandler::isUnsignedType(const Type &type) {
    return StringUtils::startsWith(type.baseType(), "unsigned");
}

bool types::TypesHandler::isIntegerType(const TypeName &typeName) {
    return CollectionUtils::containsKey(integerTypesToSizes, typeName);
}

/*
 * Floating point types
 */
static const std::unordered_map<std::string, size_t> floatingPointTypesToSizes = {
        {"float",       SizeUtils::bytesToBits(sizeof(float))},
        {"double",      SizeUtils::bytesToBits(sizeof(double))},
        {"long double", SizeUtils::bytesToBits(sizeof(long double))}
};

bool types::TypesHandler::isFloatingPointType(const Type &type) {
    return type.isSimple() && isFloatingPointType(type.baseType());
}

bool types::TypesHandler::isFloatingPointType(const TypeName &type) {
    return CollectionUtils::containsKey(floatingPointTypesToSizes, type);
}

/*
 * Character types
 */
static const std::unordered_map<std::string, size_t> characterTypesToSizes = {
        {"char",          SizeUtils::bytesToBits(sizeof(char))},
        {"signed char",   SizeUtils::bytesToBits(sizeof(signed char))},
};

bool types::TypesHandler::isCharacterType(const Type &type) {
    return type.isSimple() && isCharacterType(type.baseType());
}

bool types::TypesHandler::isCharacterType(const TypeName &type) {
    return CollectionUtils::containsKey(characterTypesToSizes, type);
}

/*
 * Boolean types
 */
static const std::unordered_map<std::string, size_t> boolTypesToSizes = {
        {"bool",  SizeUtils::bytesToBits(sizeof(bool))},
        {"_Bool", SizeUtils::bytesToBits(sizeof(bool))}
};

bool types::TypesHandler::isBoolType(const Type &type) {
    return type.isSimple() && isBoolType(type.baseType());
}

bool types::TypesHandler::isBoolType(const TypeName &type) {
    return CollectionUtils::containsKey(boolTypesToSizes, type);
}

/*
 * All primitive types
 */
bool types::TypesHandler::isPrimitiveType(const Type &type) {
    return isIntegerType(type)
           || isFloatingPointType(type)
           || isCharacterType(type)
           || isBoolType(type)
           || isVoid(type);
}

bool types::TypesHandler::isCStringType(const Type &type) {
    return isOneDimensionPointer(type) && isCharacterType(type.baseType());
}

bool types::TypesHandler::isCppStringType(const Type &type) {
    return type.typeName() == "string";
}

/*
 * Struct types (structs and unions)
 */
bool types::TypesHandler::isStructLike(const Type &type) const {
    return type.isSimple() && isStructLike(type.getId());
}

bool types::TypesHandler::isStructLike(uint64_t id) const {
    return typeIsInMap(id, typeMaps.structs);
}

/*
 * Enum types
 */
bool types::TypesHandler::isEnum(const types::Type &type) const {
    return type.isSimple() && isEnum(type.getId());
}

bool types::TypesHandler::isEnum(uint64_t id) const {
    return typeIsInMap(id, typeMaps.enums);
}

/*
 * Void type
 */
bool types::TypesHandler::isVoid(const Type &type) {
    return type.isSimple() && isVoid(type.baseType());
}

bool types::TypesHandler::baseTypeIsVoid(const Type &type) {
    return isVoid(type.baseType());
}

bool types::TypesHandler::isVoid(const TypeName &type) {
    return type == "void";
}

bool types::TypesHandler::isPointerToFunction(const Type& type) {
    return type.isPointerToFunction();
}

bool types::TypesHandler::isArrayOfPointersToFunction(const Type& type) {
    return type.isArrayOfPointersToFunction();
}

bool types::TypesHandler::omitMakeSymbolic(const Type& type) {
    return isVoid(type) || type.isPointerToFunction() ||
           type.isArrayOfPointersToFunction() || type.isObjectPointer();
}

bool types::TypesHandler::skipTypeInReturn(const Type& type) {
    return isVoid(type) || isPointerToFunction(type) || isArrayOfPointersToFunction(type);
}

/*
 * Get struct information
 */
types::StructInfo types::TypesHandler::getStructInfo(const Type &type) const {
    return getStructInfo(type.getId());
}

types::StructInfo types::TypesHandler::getStructInfo(uint64_t id) const {
    return typeFromMap<StructInfo>(id, typeMaps.structs);
}

/*
 * Get enum information
 */
types::EnumInfo types::TypesHandler::getEnumInfo(const types::Type &type) const {
    return getEnumInfo(type.getId());
}

types::EnumInfo types::TypesHandler::getEnumInfo(uint64_t id) const {
    return typeFromMap<EnumInfo>(id, typeMaps.enums);
}

/**
 * Checks whether type is a pointer
 */
bool types::TypesHandler::isObjectPointerType(const Type &type) {
    return type.isObjectPointer();
}

/**
 * Checks whether type is an array
 */
bool types::TypesHandler::isArrayType(const Type &type) {
    return type.isArray();
}

bool types::TypesHandler::isIncompleteArrayType(const Type &type) {
    return type.isArray() && !dynamic_cast<ArrayType *>(type.kinds().front().get())->isComplete();
}

bool types::TypesHandler::isOneDimensionPointer(const types::Type &type) {
    return type.isOneDimensionPointer();
}

size_t types::TypesHandler::typeSize(const types::Type &type) const {
    if (isIntegerType(type)) {
        return integerTypesToSizes.at(type.baseType());
    }

    if (isBoolType(type)) {
        return boolTypesToSizes.at(type.baseType());
    }

    if (isFloatingPointType(type)) {
        return floatingPointTypesToSizes.at(type.baseType());
    }

    if (isCharacterType(type)) {
        return characterTypesToSizes.at(type.baseType());
    }

    if (isStructLike(type)) {
        return getStructInfo(type).size;
    }

    if (isEnum(type)) {
        return getEnumInfo(type).size;
    }

    if (isArrayType(type)) {
        size_t elementsNum = type.kinds().front()->getSize();
        size_t elementSize = typeSize(type.baseTypeObj());
        return elementSize * elementsNum;
    }

    if (isObjectPointerType(type)) {
        return SizeUtils::bytesToBits(getPointerSize());
    }

    if (isPointerToFunction(type)) {
        return SizeUtils::bytesToBits(sizeof(char *));
    }

    throw UnImplementedException("Type is unknown for: " + type.typeName());
}

std::string types::TypesHandler::removeConstPrefix(const TypeName &type) {
    std::vector<std::string> tmp = StringUtils::split(type);
    if (tmp[0] == CONST_QUALIFIER) {
        std::string res;
        for (size_t i = 1; i < tmp.size(); i++) {
            res += tmp[i];
            if (i + 1 < tmp.size()) {
                res.push_back(' ');
            }
        }
        return res;
    }
    return type;
}

bool types::TypesHandler::hasConstModifier(const types::TypeName &typeName) {
    const auto &splitType = StringUtils::splitByWhitespaces(typeName);
    return std::find(splitType.begin(), splitType.end(), CONST_QUALIFIER) != splitType.end();
}

std::string types::TypesHandler::removeArrayReference(TypeName type) {
    if (type[0] == '*') {
        type = type.substr(1, type.size());
    } else if (type.back() == '*') {
        type.pop_back();
    }
    if (type.size() > 2 && type.back() == ']' && type[type.size() - 2] == '[') {
        type.pop_back();
        type.pop_back();
    }
    StringUtils::trim(type);
    return type;
}

std::string types::TypesHandler::removeArrayBrackets(TypeName type) {
    if (type.back() == ']') {
        while (!type.empty() && type.back() != '[') {
            type.pop_back();
        }
        if (!type.empty()) {
            type.pop_back();
        }
    }

    StringUtils::trim(type);
    return type;
}

testsgen::ValidationType types::TypesHandler::getIntegerValidationType(const Type &type) {
    size_t size;
    if (isIntegerType(type)) {
        size = integerTypesToSizes.at(type.baseType());
    } else {
        ABORT_F("type is not an integerType: %s", type.baseType().c_str());
    }
    bool isUnsigned = isUnsignedType(type);
    if (size == 8) {
        return (isUnsigned) ? testsgen::UINT8_T : testsgen::INT8_T;
    } else if (size == 16) {
        return (isUnsigned) ? testsgen::UINT16_T : testsgen::INT16_T;
    } else if (size == 32) {
        return (isUnsigned) ? testsgen::UINT32_T : testsgen::INT32_T;
    } else if (size == 64) {
        return (isUnsigned) ? testsgen::UINT64_T : testsgen::INT64_T;
    } else {
        ABORT_F("Unknown integer size: %zu", size);
    }
}

const std::unordered_map<types::TypeName, std::vector<std::string>> &types::TypesHandler::preferredConstraints() noexcept {
    static const std::unordered_map<std::string, std::vector<std::string>> constraints = {
            {"char",               {">= 'a'",  "<= 'z'", "!= '\\0'"}},
            {"signed char",        {">= 'a'",  "<= 'z'", "!= '\\0'"}},
            {"unsigned char",      {">= 'a'",  "<= 'z'", "!= '\\0'"}},
            {"short",              {">= -10", "<= 10"}},
            {"int",                {">= -10", "<= 10"}},
            {"long",               {">= -10", "<= 10"}},
            {"long long",          {">= -10", "<= 10"}},
            {"unsigned short",     {"<= 10"}},
            {"unsigned int",       {"<= 10"}},
            {"unsigned long",      {"<= 10"}},
            {"unsigned long long", {"<= 10"}},
            {"float",              {">= -10", "<= 10"}},
            {"double",             {">= -10", "<= 10"}},
            {"long double",        {">= -10", "<= 10"}},
            {"void",               {"<= 10"}},
    };

    return constraints;
}

types::TypeKind types::TypesHandler::getTypeKind(const Type &type) const {
    if (isPrimitiveType(type)) {
        return TypeKind::PRIMITIVE;
    }

    if (isObjectPointerType(type)) {
        return TypeKind::OBJECT_POINTER;
    }

    if (isArrayType(type)) {
        return TypeKind::ARRAY;
    }

    if (isStructLike(type)) {
        return TypeKind::STRUCT_LIKE;
    }

    if (isEnum(type)) {
        return TypeKind::ENUM;
    }

    if (isPointerToFunction(type)) {
        return TypeKind::FUNCTION_POINTER;
    }

    return TypeKind::UNKNOWN;
}

std::string types::TypesHandler::getDefaultValueForType(const types::Type &type,
                                                        utbot::Language language) const {
    if (isIntegerType(type)) {
        return "0";
    }

    if (isBoolType(type)) {
        if (language == utbot::Language::CXX) {
            return "false";
        } else {
            return "0";
        }
    }

    if (isFloatingPointType(type)) {
        return ".0";
    }

    if (isVoid(type)) {
        return "";
    }

    if (isCharacterType(type)) {
        return "'\\0'";
    }

    if (isArrayType(type)) {
        return "{}";
    }

    if (isCStringType(type)) {
        return "\"\"";
    }

    if (isObjectPointerType(type)) {
        return PrinterUtils::C_NULL;
    }

    TypeName name = type.typeName();
    std::string cDefaultValue = StringUtils::stringFormat("(%s){0}", name);
    std::string cppDefaultValue = "{}";
    switch (language) {
    case utbot::Language::C: {
        LOG_S(WARNING) << "Couldn't determine kind of type while generating default value. Using "
                          "\"(%s){0}\" instead.";
        return cDefaultValue;
    }
    case utbot::Language::CXX: {
        LOG_S(WARNING) << "Couldn't determine kind of type while generating default value. Using "
                          "\"{}\" instead.";
        return cppDefaultValue;
    }
    default: {
        LOG_S(WARNING) << "Unknown language for getDefaultValueForType. Using ifdef macro";
        return StringUtils::stringFormat("\n#ifdef __cplusplus\n %s\n #else\n %s\n #endif\n",
                                         cppDefaultValue, cDefaultValue);
    }
    }
}

std::string types::TypesHandler::cBoolToCpp(const types::TypeName &type) {
    return isBoolType(type) ? "bool" : type;
}

types::TypeSupport
types::TypesHandler::isSupportedType(const Type &type, TypeUsage usage, int depth) const {
    auto hashArgument = IsSupportedTypeArguments(type.typeName(), usage);
    auto writtenValue = isSupportedTypeHash.find(hashArgument);
    if (writtenValue != isSupportedTypeHash.end()) {
        return writtenValue->second;
    }
    recursiveCheckStarted.insert(type.typeName());
    using PredicateWithReason = std::pair<std::string, std::function<bool(Type, TypeUsage)>>;
    std::vector<PredicateWithReason> unsupportedPredicates = {
        {
            "Type is unknown",
            [&](const Type &type, TypeUsage usage) {
              return getTypeKind(type) == TypeKind::UNKNOWN;
            }
        },
        {
            "Type has flexible array member",
            [&](const Type &type, TypeUsage usage) {
              if (isStructLike(type)) {
                  auto structInfo = getStructInfo(type);
                  if (structInfo.fields.empty()) {
                      return false;
                  }
                  return isIncompleteArrayType(structInfo.fields.back().type);
              }
              return false;
            } },
        {
            "Dimension of pointer is too big",
            [&](const Type &type, TypeUsage usage) {
              if (usage == TypeUsage::RETURN) {
                  return false;
              }
              size_t counter = 0;
              for (const auto& kind: type.kinds()) {
                  counter += kind->getKind() == AbstractType::OBJECT_POINTER ? 1: 0;
              }
              return counter > 2;
            }
        },
        { "Two dimensional pointer has extra const qualifiers",
            [&](const Type &type, TypeUsage usage) {
              if (usage == TypeUsage::RETURN) {
                  return false;
              }
              if (type.isPointerToPointer()) {
                  if (auto firstPointer =
                      dynamic_cast<ObjectPointerType *>(type.kinds()[0].get())) {
                      if (firstPointer->isConstQualified()) {
                          return true;
                      }
                      if (auto secondPointer =
                          dynamic_cast<ObjectPointerType *>(type.kinds()[1].get())) {
                          if (secondPointer->isConstQualified()) {
                              return true;
                          }
                      }
                  }
              }
              return false;
            } },
        {
            "Unsupported types in structs/unions",
            [&](const Type &type, TypeUsage usage) {
              auto unsupportedFields = [&](const std::vector<types::Field> &fields) {
                return std::any_of(fields.begin(), fields.end(), [&](const types::Field &field) {
                  if (!CollectionUtils::contains(recursiveCheckStarted, field.type.typeName())) {
                      if (field.type.isObjectPointer()) {
                          return false;
                      }
                      auto support = isSupportedType(field.type, usage, depth + 1);
                      bool fieldSupported = support.isSupported;
                      return !fieldSupported;
                  }
                  return false;
                });
              };
              if (isStructLike(type)) {
                  auto structInfo = getStructInfo(type);
                  return unsupportedFields(structInfo.fields);
              }
              return false;
            }
        },
        { "Base type of array or pointer",
            [&](const Type &type, TypeUsage usage) {
              if (type.isArray() || type.isObjectPointer()) {
                  if (type.isObjectPointer() && depth > 0) {
                      return false;
                  }
                  auto support = isSupportedType(type.baseTypeObj(), usage, depth + 1);
                  bool supported = support.isSupported;
                  return !supported;
              }
              return false;
            } }
    };

    for (const auto &[reason, predicate]: unsupportedPredicates) {
        if (predicate(type, usage)) {
            recursiveCheckStarted.erase(type.typeName());
            return {false, reason};
        }
    }
    recursiveCheckStarted.erase(type.typeName());
    types::TypeSupport result = {true, ""};
    isSupportedTypeHash[IsSupportedTypeArguments(hashArgument)] = result;
    return result;
}


types::TypesHandler::IsSupportedTypeArguments::IsSupportedTypeArguments(types::TypeName typeName,
                                                                        types::TypeUsage usage)
    : typeName(std::move(typeName)), usage(usage) {
}

bool types::TypesHandler::IsSupportedTypeArguments::operator==(const types::TypesHandler::IsSupportedTypeArguments &other) const {
    return typeName == other.typeName && usage == other.usage;
}

std::size_t types::TypesHandler::IsSupportedTypeArgumentsHash::operator()(const types::TypesHandler::IsSupportedTypeArguments &args) const {
    return fs::hash_value(args.typeName + std::to_string((int)args.usage));
}

types::Type types::TypesHandler::getReturnTypeToCheck(const types::Type &returnType) const {
    types::Type baseReturnType = returnType.baseTypeObj();
    if (types::TypesHandler::isObjectPointerType(returnType)) {
        if (types::TypesHandler::skipTypeInReturn(baseReturnType)) {
            return types::Type::minimalScalarType();
        }
        return baseReturnType;
    }
    return returnType;
}

std::string types::EnumInfo::getEntryName(const std::string &value, utbot::Language language) const {
    const EnumEntry &entry = valuesToEntries.at(value);
    if (language == utbot::Language::CXX && access.has_value()) {
        return access.value() + "::" + entry.name;
    }
    return entry.name;
}
