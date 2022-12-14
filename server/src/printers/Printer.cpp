#include "Printer.h"

#include "NameDecorator.h"
#include "types/SimpleType.h"
#include "utils/ArgumentsUtils.h"
#include "utils/Copyright.h"
#include "utils/StubsUtils.h"
#include "visitors/VerboseParameterVisitor.h"
#include "printers/KleeConstraintsPrinter.h"

#include "loguru.h"

namespace printer {
    using StringUtils::stringFormat;

    Printer::Printer(utbot::Language srcLanguage) : srcLanguage(srcLanguage){
    }

    bool Printer::needDecorate() const {
        return (getLanguage() == utbot::Language::CXX) && (srcLanguage != utbot::Language::CXX);
    }

    void printer::Printer::resetStream() {
        ss.str("");
        ss.clear();
        tabsDepth = 0;
    }

    std::string printer::Printer::LB(bool startsWithSpace) {
        tabsDepth++;
        return std::string(startsWithSpace ? " " : "") + "{\n";
    }

    std::string printer::Printer::RB(bool needSC) {
        tabsDepth--;
        return LINE_INDENT() + "}" + (needSC ? ";" : "") + "\n";
    }

    Printer::Stream &printer::Printer::strInclude(const std::string &header, bool isAngled) {
        char begin = isAngled ? '<' : '\"';
        char end = isAngled ? '>' : '\"';
        ss << LINE_INDENT() << "#include " << begin << header << end << NL;
        return ss;
    }

    std::stringstream &printer::Printer::strDefine(std::string_view from, std::string_view to) {
        ss << "#define " << from << " " << to << NL;
        return ss;
    }

    std::stringstream &printer::Printer::strInclude(const Include &include) {
        return strInclude(include.path, include.is_angled);
    }

    Printer::Stream &printer::Printer::strIncludeSystem(const std::string &header) {
        ss << LINE_INDENT() << "#include <" << header << ">" << NL;
        return ss;
    }

    Printer::Stream &printer::Printer::strForBound(const std::string &it, size_t n) {
        ss << LINE_INDENT() << "for (int " << it << " = 0; " << it << " < " << n << "; " << it << " ++)";
        return ss;
    }

    Printer::Stream &printer::Printer::strIfBound(const std::string &condition) {
        ss << LINE_INDENT() << "if (" << condition << ")";
        return ss;
    }

    std::vector<std::string>
    printer::Printer::printForLoopsAndReturnLoopIterators(const std::vector<size_t> &bounds) {
        thread_local int counter = 0;
        counter++;

        std::vector<std::string> iterators;
        for (size_t i = 0; i < bounds.size(); ++i) {
            std::string it = StringUtils::stringFormat("it_%d_%d", counter, i);
            iterators.push_back(it);
            strForBound(it, bounds[i]) << LB();
        }

        return iterators;
    }


    Printer::Stream &printer::Printer::strDeclareVar(std::string_view type,
                                                     std::string_view name,
                                                     std::optional<std::string_view> initValue,
                                                     std::optional<uint64_t> alignment,
                                                     bool complete,
                                                     size_t additionalPointersCount) {
        ss << LINE_INDENT();
        printAlignmentIfExists(alignment);
        auto additionalPointers = StringUtils::repeat("*", additionalPointersCount);
        if (needDecorate()) {
            ss << NameDecorator::decorate(type) << additionalPointers << " "
               << NameDecorator::decorate(name);
        } else if (getLanguage() == utbot::Language::CXX) {
            ss << types::TypesHandler::cBoolToCpp(std::string(type)) << additionalPointers << " " << name;
        } else {
            ss << type << additionalPointers << " " << name;
        }
        if (initValue.has_value()) {
            ss << " = " << initValue.value();
        }
        if (complete) {
            ss << SCNL;
        }
        return ss;
    }
    void Printer::printAlignmentIfExists(const std::optional<uint64_t> &alignment) {
        if (alignment.has_value()) {
            ss << stringFormat("__attribute__ ((aligned(%llu)))", alignment) << " ";
        }
    }

    Printer::Stream &printer::Printer::strDeclareAbsError(const std::string &name) {
        ss << LINE_INDENT() << "static const float " << name << " = 1e-6;" << NL;
        return ss;
    }

    Printer::Stream &printer::Printer::strDeclareArrayVar(const types::Type &type,
                                                          std::string_view name,
                                                          types::PointerUsage usage,
                                                          std::optional<std::string_view> value,
                                                          std::optional<uint64_t> alignment,
                                                          bool complete) {
        auto baseType = type.baseType();
        std::string arrayName{ name.data(), name.length() };

        if (needDecorate()) {
            baseType = NameDecorator::decorate(baseType);
            arrayName = NameDecorator::decorate(arrayName);
        }

        ss << LINE_INDENT();
        printAlignmentIfExists(alignment);
        ss << baseType << " " << arrayName;
        std::vector<size_t> sizes = type.arraysSizes(usage);
        bool isLiteral = sizes.size() == 1 &&
                         types::TypesHandler::isCharacterType(type.baseTypeObj()) &&
                         value.has_value();
        if (isLiteral) {
            ss << "[]";
        } else {
            for (auto size : sizes) {
                ss << "[" << size << "]";
            }
        }
        if (value.has_value()) {
            ss << " = " << value.value();
        }
        if (complete) {
            ss << SCNL;
        }
        return ss;
    }


    Printer::Stream &Printer::strAssignVar(std::string_view name, std::string_view value) {
        if (needDecorate()) {
            ss << LINE_INDENT() << NameDecorator::decorate(name) << " = " << value << SCNL;
        } else {
            ss << LINE_INDENT() << name << " = " << value << SCNL;
        }
        return ss;
    }

    Printer::Stream &Printer::strTabIf(bool needTabs) {
        ss << (needTabs ? LINE_INDENT() : "");
        return ss;
    }

    Printer::Stream &Printer::strFunctionDecl(
        const std::string &returnType,
        const std::string &functionName,
        const std::vector<types::Type> &paramTypes,
        const std::vector<std::string> &paramValues,
        const std::string &end,
        const std::vector<std::string> &modifiers,
        const tests::Tests::MethodDescription::FPointerMap &fullDeclSubstitutions,
        bool isVariadic) {
        ss << LINE_INDENT();
        for (const auto &modifier : modifiers) {
            ss << modifier << " ";
        }
        ss << returnType << " " << functionName << "(";
        auto n = std::min(paramTypes.size(), paramValues.size());
        for (auto i = 0; i < n; i++) {
            bool named = false;
            if (CollectionUtils::containsKey(fullDeclSubstitutions, paramValues[i])) {
                ss << getTypedefFunctionPointer(functionName, paramValues[i], paramTypes[i].isArrayOfPointersToFunction());
            } else {
                if (paramTypes[i].isPointerToArray()) {
                    auto decomposedType = StringUtils::split(paramTypes[i].usedType(), '*');
                    /*
                     * code example
                     * typedef int int_array[1];
                     * int_array* int_array_pointer;
                     * usedType == int_array*
                     * mTypeName == int_array*[1]
                     */
                    if (decomposedType.size() != 2) {
                        decomposedType = StringUtils::split(paramTypes[i].mTypeName(), '*');
                    }
                    LOG_IF_S(ERROR, decomposedType.size() != 2) << "Type::isPointerToArray malfunction";
                    ss << decomposedType[0] << "*" << paramValues[i] << decomposedType[1];
                    named = true;
                } else {
                    ss << paramTypes[i].usedType();
                }
            }
            if (!named && !paramValues[i].empty()) {
                ss << " " << paramValues[i];
            }
            if (i != n - 1) {
                ss << ", ";
            }
        }
        if (isVariadic) {
            ss << ", ...";
        }
        ss << ")" << end;
        return ss;
    }

    Printer::Stream &Printer::strFunctionDecl(const tests::Tests::MethodDescription &method,
                                              const std::string &end,
                                              const std::vector<std::string> &modifiers) {
        return strFunctionDecl(method.returnType.usedType(), method.name, method.getParamTypes(),
                               method.getParamNames(), end, modifiers, method.functionPointers,
                               method.isVariadic);
    }


    Printer::Stream &
    Printer::strFunctionDeclWithParamString(const tests::Tests::MethodDescription &method,
                                            const std::string &end,
                                            const std::vector<std::string> &modifiers) {
        ss << LINE_INDENT();
        for (const auto &modifier : modifiers) {
            ss << modifier << " ";
        }
        ss << method.returnType.usedType() << " " << method.name << "(" << method.paramsString
           << ")" << end;
        return ss;
    }

    Printer::Stream &Printer::strFunctionCall(std::string_view functionName,
                                              const std::vector<std::string> &args,
                                              const std::string &end,
                                              const std::optional<std::string> &classObj,
                                              bool needTabs,
                                              size_t retPointers,
                                              std::optional<types::Type> castType,
                                              bool needComment) {
        if (needComment) {
            ss << "//";
        }
        strTabIf(needTabs);
        for (size_t i = 0; i < retPointers; ++i) {
            ss << "*";
        }
        if (castType.has_value()) {
            ss << "(" << castType->typeName() << ")";
        }
        std::string methodName(functionName);
        if (needDecorate()) {
            methodName = NameDecorator::decorate(functionName);
        }
        if (classObj.has_value()) {
            methodName = classObj.value() + "." + methodName;
        }
        ss << methodName << "(" << StringUtils::joinWith(args, ", ") << ")" << end;
        return ss;
    }

    Printer::Stream &Printer::strFunctionCall(const tests::Tests::MethodDescription &method,
                                              size_t returnPointers,
                                              const std::string &end,
                                              bool needTabs) {
        strTabIf(needTabs);
        std::vector<std::string> parameters;
        for (const auto &param : method.params) {
            parameters.push_back(param.getFunctionParamDecl());
        }
        auto classObjName = method.getClassName();
        return strFunctionCall(method.name, parameters, end, classObjName, needTabs,
                               returnPointers);
    }

    Printer::Stream &Printer::strComment(const std::string &comment) {
        ss << LINE_INDENT() << "// " << comment << NL;
        return ss;
    }

    Printer::Stream &Printer::commentBlockSeparator() {
        ss << LINE_INDENT() << "//////////////////////////////////////////// " << NL;
        return ss;
    }

    std::string Printer::constrIndex(const std::string &arrayName, const std::string &ind) {
        return arrayName + "[" + ind + "]";
    }

    std::string Printer::constrIndex(const std::string &arrayName, int ind) {
        return constrIndex(arrayName, std::to_string(ind));
    }

    std::string Printer::constrMultiIndex(const std::string &arrayName, const std::vector<std::string> &indexes) {
        std::string element = arrayName;
        for (const auto &index : indexes) {
            element = constrIndex(element, index);
        }
        return element;
    }

    std::string Printer::constrMultiIndex(const std::string &arrayName, const std::vector<size_t> &indexes) {
        std::vector<std::string> strIndexes;
        for (size_t index : indexes) {
            strIndexes.push_back(std::to_string(index));
        }
        return constrMultiIndex(arrayName, strIndexes);
    }

    std::string Printer::constrMultiIndex(const std::vector<std::string> &indexes) {
        return constrMultiIndex("", indexes);
    }

    std::string Printer::constrFunctionCall(const tests::Tests::MethodDescription &method,
                                            size_t returnPointers,
                                            const std::string &end,
                                            bool needTabs) {
        std::stringstream func_ss;
        ss.swap(func_ss);
        strFunctionCall(method, returnPointers, end, needTabs);
        ss.swap(func_ss);
        return func_ss.str();
    }

    std::string Printer::constrFunctionCall(const std::string &functionName,
                                            const std::vector<std::string> &args,
                                            const std::string &end,
                                            const std::optional<std::string> &classObjName,
                                            bool needTabs,
                                            size_t retPointers,
                                            std::optional<types::Type> castType) {
        std::stringstream func_ss;
        ss.swap(func_ss);
        strFunctionCall(functionName, args, end, classObjName, needTabs, retPointers,
                        std::move(castType));
        ss.swap(func_ss);
        return func_ss.str();
    }


    Printer::Stream &Printer::writeCodeLine(std::string_view str) {
        ss << LINE_INDENT() << str << SCNL;
        return ss;
    }

    std::string Printer::recursiveIteratorName(const std::string &prefix) const {
        return prefix + std::to_string(tabsDepth);
    }

    const std::string MEMCPY = "memcpy";
    const std::string SIZEOF = "sizeof";

    Printer::Stream
    Printer::strMemcpy(std::string_view dest, std::string_view src, bool needDereference) {
        if (needDecorate()) {
            return strMemcpyImpl(NameDecorator::decorate(dest), NameDecorator::decorate(src),
                                 needDereference);
        } else {
            return strMemcpyImpl(dest, src, needDereference);
        }
    }

    Printer::Stream &Printer::strReturn(std::string_view value) {
        ss << LINE_INDENT();
        if (value.empty()) {
            ss << "return";
        } else {
            ss << "return " << value;
        }
        ss << SCNL;
        return ss;
    }

    std::stringstream& Printer::checkOverflowStubArray(const std::string &cntCall) {
        ss << LINE_INDENT() << "if (" << cntCall << " == " <<
           types::TypesHandler::getElementsNumberInPointerOneDim(types::PointerUsage::PARAMETER) << ") {" << NL;
        tabsDepth++;
        ss << LINE_INDENT() << cntCall << "--;" << NL;
        ss << RB();
        return ss;
    }


    std::stringstream &Printer::strStubForMethod(const Tests::MethodDescription &method,
                                                 const types::TypesHandler &typesHandler,
                                                 const std::string &prefix,
                                                 const std::string &suffix,
                                                 const std::string &methodName,
                                                 const std::string &nameForStub,
                                                 bool makeStatic) {
        auto methodCopy = method;
        methodCopy.name = method.name;

        std::string stubSymbolicVarName = getStubSymbolicVarName(nameForStub);
        if (!types::TypesHandler::omitMakeSymbolic(method.returnType)) {
            stubSymbolicVarName = getStubSymbolicVarName(methodName + "_" + nameForStub);
            strDeclareArrayVar(types::Type::createArray(method.returnType), stubSymbolicVarName,
                               types::PointerUsage::PARAMETER);
        }

        if (!prefix.empty()) {
            methodCopy.name = prefix + "_" + methodCopy.name;
        }
        if (!suffix.empty()) {
            methodCopy.name += "_" + suffix;
        }
        std::vector<std::string> modifiers;
        if (makeStatic) {
            modifiers.emplace_back("static");
        }
        strFunctionDecl(methodCopy, " ", modifiers) << LB(false);
        std::string returnValue;
        if (types::TypesHandler::omitMakeSymbolic(method.returnType)) {
            returnValue = typesHandler.getDefaultValueForType(methodCopy.returnType, getLanguage());
            strReturn(returnValue) << RB() << NL;
            return ss;
        }

        std::string firstTimeCallVar = "firstTimeCall";
        strDeclareVar("static int", firstTimeCallVar, "1");
        const std::string cntCall = "cntCall";
        strDeclareVar("static int", cntCall, "0");
        ss << LINE_INDENT() << "#ifdef " << PrinterUtils::KLEE_MODE << NL;
        tabsDepth++;
        ss << LINE_INDENT() << "if (" << firstTimeCallVar << " == 1)" << LB();
        strAssignVar(firstTimeCallVar, "0");
        strKleeMakeSymbolic(stubSymbolicVarName, !method.returnType.isArray(),
                            stubSymbolicVarName);
        types::TypeMaps tempMap = {};
        auto temp = std::make_shared<types::TypesHandler>(tempMap, types::TypesHandler::SizeContext());
        printer::KleeConstraintsPrinter preferWriter(temp.get(), srcLanguage);
        preferWriter.setTabsDepth(tabsDepth);
        preferWriter.genConstraints(
            {types::Type::createArray(method.returnType), stubSymbolicVarName, std::nullopt});
        ss << preferWriter.ss.str();
        ss << RB();
        tabsDepth--;
        ss << LINE_INDENT() << "#endif" << NL;

        checkOverflowStubArray(cntCall);

        returnValue = stubSymbolicVarName + "[" + cntCall + "++]";
        strReturn(returnValue) << RB() << NL;
        return ss;
    }

    std::string Printer::getStubSymbolicVarName(const std::string &methodName) {
        return methodName + PrinterUtils::KLEE_SYMBOLIC_SUFFIX;
    }

    Printer::Stream Printer::strKleeMakeSymbolic(const std::string &varName, bool needAmpersand, SRef pseudoName) {
        auto pointer = (needAmpersand ? "&" : "") + varName;
        auto size = "sizeof(" + varName + ")";
        auto name = "\"" + pseudoName + "\"";
        strFunctionCall("klee_make_symbolic", { pointer, size, name });
        return ss;
    }

    std::stringstream &Printer::strDeclareArrayOfFunctionPointerVar(
        const std::string &arrayType, const std::string &arrayName, const std::string &stubFunctionName) {
        size_t size =
            types::TypesHandler::getElementsNumberInPointerOneDim(types::PointerUsage::PARAMETER);
        strDeclareVar(arrayType, arrayName + "[" + std::to_string(size) + "]");
        strForBound("i", size) << " " << BNL;
        tabsDepth++;
        strAssignVar(arrayName + "[i]", stubFunctionName);
        tabsDepth--;
        ss << LINE_INDENT() << "}" << NL;
        return ss;
    }

    std::stringstream &Printer::strTypedefFunctionPointer(const types::FunctionInfo &method,
                                                          const std::string &name) {
        auto paramTypes =
            CollectionUtils::transform(method.params, [](const auto &param) { return param.type; });
        ss << LINE_INDENT() << "typedef ";
        strFunctionDecl(method.returnType.usedType(), StringUtils::stringFormat("(*%s)", name), paramTypes,
                        std::vector<std::string>(paramTypes.size(), ""), "") << SCNL;
        if (method.isArray) {
            ss << LINE_INDENT() << "typedef ";
            strFunctionDecl(method.returnType.usedType(), StringUtils::stringFormat("(**%s)", name + "_arr"), paramTypes,
                            std::vector<std::string>(paramTypes.size(), ""), "") << SCNL;
        }
        return ss;
    }

    std::stringstream &Printer::closeBrackets(size_t sz) {
        for (size_t i = 0; i < sz; ++i) {
            ss << RB();
        }
        return ss;
    }

    std::stringstream &Printer::gen2DPointer(const Tests::MethodParam &param, bool needDeclare) {
        if (!param.type.isPointerToPointer()) {
            return ss;
        }

        size_t pointerSize = types::TypesHandler::getElementsNumberInPointerMultiDim(types::PointerUsage::PARAMETER);
        auto typeObject = types::TypesHandler::isVoid(param.type.baseTypeObj())
                              ? types::Type::minimalScalarPointerType(2)
                              : param.type;
        auto baseType = typeObject.baseType();
        auto type = stringFormat("%s%s **", getConstQualifier(typeObject), baseType);
        std::string value =
            stringFormat("(%s) calloc(%zu, sizeof(%s *))", type, pointerSize + 1, baseType);
        if (needDeclare) {
            strDeclareVar(type, param.name, value);
        } else {
            strAssignVar(param.name, value);
        }

        auto iterators = printForLoopsAndReturnLoopIterators({ pointerSize });
        auto indexing = constrMultiIndex(iterators);
        strAssignVar(param.name + indexing, param.underscoredName() + indexing);
        closeBrackets(1);
        strAssignVar(constrIndex(param.name, pointerSize), PrinterUtils::C_NULL);
        return ss;
    }

    Printer::Stream
    Printer::strMemcpyImpl(std::string_view dest, std::string_view src, bool needDereference) {
        using namespace std::string_literals;
        std::string destArg = stringFormat("(void *) %s%.*s", (needDereference ? "&"s : ""s),
                                      dest.length(), dest.data());
        std::string count = stringFormat("%s(%.*s)", SIZEOF, src.length(), src.data());
        strFunctionCall(MEMCPY, { destArg, std::string(src), count });
        return ss;
    }

    void printer::Printer::writeStubsForFunctionParams(const types::TypesHandler *typesHandler,
                                                       const Tests::MethodDescription &testMethod,
                                                       bool forKlee) {
        std::string scopeName = (forKlee ? testMethod.getClassName().value_or("") : "");
        std::string prefix = PrinterUtils::getKleePrefix(forKlee);
        for (const auto &[name, pointerFunctionStub] : testMethod.functionPointers) {
            std::string stubName = StubsUtils::getFunctionPointerStubName(scopeName, testMethod.name, name, true);
            writeStubForParam(typesHandler, pointerFunctionStub, testMethod.name, stubName, true,
                              forKlee);
        }
    }

    void printer::Printer::writeExternForSymbolicStubs(const Tests::MethodDescription& testMethod) {
        std::unordered_map<std::string, std::string> symbolicNamesToTypesMap;
        for (const auto& testCase: testMethod.testCases) {
            for (size_t i = 0; i < testCase.stubValues.size(); i++) {
                symbolicNamesToTypesMap[testCase.stubValues[i].name] = testCase.stubValuesTypes[i].type.usedType();
            }
        }
        for (const auto& [name, type]: symbolicNamesToTypesMap) {
            strDeclareVar("extern \"C\" " + type, name);
        }
    }

    void printer::Printer::writeStubForParam(const types::TypesHandler *typesHandler,
                                             const std::shared_ptr<types::FunctionInfo> &fInfo,
                                             const std::string &methodName,
                                             const std::string &stubName,
                                             bool needToTypedef,
                                             bool makeStatic) {
        if (needToTypedef) {
            auto typedefName = getTypedefFunctionPointer(methodName, fInfo->name, false);
            strTypedefFunctionPointer(*fInfo, typedefName);
        }
        strStubForMethod(tests::Tests::MethodDescription::fromFunctionInfo(*fInfo), *typesHandler,
                         stubName, "stub", methodName, fInfo->name, makeStatic);
    }

    void Printer::writeAccessPrivateMacros(types::TypesHandler const *typesHandler, const Tests &tests, bool onlyChangeable) {
        if (srcLanguage == utbot::Language::CXX) {
            ss << NL;
            strInclude("access_private.hpp");
            ss << NL;
            std::unordered_set<uint64_t> checkedOnPrivate;
            for (const auto &[methodName, testMethod] : tests.methods) {
                addAccessor(typesHandler, testMethod.returnType, checkedOnPrivate);
                if (testMethod.isClassMethod()) {
                    addAccessor(typesHandler, testMethod.classObj->type, checkedOnPrivate);
                }
                for (const auto& param : testMethod.params) {
                    if (!onlyChangeable || param.isChangeable()) {
                        addAccessor(typesHandler, param.type, checkedOnPrivate);
                    }
                }
            }
            ss << NL;
        }
    }

    void Printer::addAccessor(const types::TypesHandler *typesHandler, const types::Type &type,
                              std::unordered_set<uint64_t> &checkedOnPrivate) {
        if (!checkedOnPrivate.count(type.getId()) && typesHandler->isStructLike(type)) {
            checkedOnPrivate.insert(type.getId());
            for (const auto& field : typesHandler->getStructInfo(type).fields) {
                if (field.accessSpecifier != types::Field::AS_pubic) {
                    ss << StringUtils::stringFormat("ACCESS_PRIVATE_FIELD(%s, %s, %s)",
                                                    type.typeName(),
                                                    field.type.typeName(),
                                                    field.name);
                    ss << NL;
                }
                addAccessor(typesHandler, field.type, checkedOnPrivate);
            }
        }
    }

    void Printer::genStubForStructFunctionPointer(const std::string &structName,
                                                  const types::Field &field,
                                                  const std::string &stubName) {
        std::string name = PrinterUtils::getFieldAccess(structName, field);
        strAssignVar(name, stubName);
    }

    void Printer::genStubForStructFunctionPointerArray(const std::string &structName,
                                                       const types::Field &field,
                                                       const std::string &stubName) {
        size_t size =
            types::TypesHandler::getElementsNumberInPointerOneDim(types::PointerUsage::PARAMETER);
        strForBound("i", size) << " " << BNL;
        tabsDepth++;
        std::string name = structName + "." + field.name + "[i]";
        strAssignVar(name, stubName);
        tabsDepth--;
        ss << LINE_INDENT() << "}" << NL;
    }

    void Printer::writeStubsForStructureFields(const Tests &tests) {
        if (!tests.stubs.empty()) {
            ss << tests.stubs << NL;
        }
    }

    void Printer::writeStubsForParameters(const Tests &tests) {
        for (const auto &[methodName, methodDescription] : tests.methods) {
            if (methodDescription.stubsText.empty()) {
                continue;
            }
            ss << methodDescription.stubsText << NL;
        }
    }

    utbot::Language Printer::getLanguage() const {
        return srcLanguage;
    }

    std::string Printer::getConstQualifier(const types::Type& type) {
        std::string constQualifier;
        if (auto simpleType = dynamic_cast<SimpleType *>(type.kinds().back().get())) {
            if (simpleType->isConstQualified()) {
                constQualifier = "const ";
            }
        }
        return constQualifier;
    }

    void Printer::writeCopyrightHeader() {
        ss << Copyright::GENERATED_C_CPP_FILE_HEADER << NL;
    }

    Printer::Stream Printer::strDeclareSetOfVars(const std::set<Tests::TypeAndVarName> &vars) {
        for (const auto &var : vars) {
            if (var.type.isArray()) {
                strDeclareArrayVar(var.type, var.varName, types::PointerUsage::KNOWN_SIZE);
            } else {
                strDeclareVar(var.type.mTypeName(), var.varName);
            }
        }
        return ss;
    }
}
