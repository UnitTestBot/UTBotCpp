/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Printer.h"

#include "NameDecorator.h"
#include "types/SimpleType.h"
#include "utils/ArgumentsUtils.h"
#include "utils/Copyright.h"
#include "visitors/VerboseParameterVisitor.h"
#include "printers/KleeConstraintsPrinter.h"

#include "loguru.h"

namespace printer {
    using StringUtils::stringFormat;
    using namespace std::string_literals;

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

    string printer::Printer::LB(bool startsWithSpace) {
        tabsDepth++;
        return string(startsWithSpace ? " " : "") + "{\n";
    }

    string printer::Printer::RB(bool needSC) {
        tabsDepth--;
        return TAB_N() + "}" + (needSC ? ";" : "") + "\n";
    }

    Printer::Stream &printer::Printer::strInclude(const string &header, bool isAngled) {
        char begin = isAngled ? '<' : '\"';
        char end = isAngled ? '>' : '\"';
        ss << TAB_N() << "#include " << begin << header << end << NL;
        return ss;
    }

    std::stringstream &printer::Printer::strDefine(std::string_view from, std::string_view to) {
        ss << "#define " << from << " " << to << NL;
        return ss;
    }

    std::stringstream &printer::Printer::strInclude(const Include &include) {
        return strInclude(include.path, include.is_angled);
    }

    Printer::Stream &printer::Printer::strIncludeSystem(const string &header) {
        ss << TAB_N() << "#include <" << header << ">" << NL;
        return ss;
    }

    Printer::Stream &printer::Printer::strForBound(const string &it, size_t n) {
        ss << TAB_N() << "for (int " << it << " = 0; " << it << " < " << n << "; " << it << " ++)";
        return ss;
    }

    Printer::Stream &printer::Printer::strIfBound(const string &condition) {
        ss << TAB_N() << "if (" << condition << ")";
        return ss;
    }

    std::vector<string>
    printer::Printer::printForLoopsAndReturnLoopIterators(const std::vector<size_t> &bounds) {
        thread_local int counter = 0;
        counter++;

        std::vector<string> iterators;
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
        ss << TAB_N();
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

    Printer::Stream &printer::Printer::strDeclareAbsError(const string &name) {
        ss << TAB_N() << "static const float " << name << " = 1e-6;" << NL;
        return ss;
    }

    Printer::Stream &printer::Printer::strDeclareArrayVar(const types::Type &type,
                                                          std::string_view name,
                                                          types::PointerUsage usage,
                                                          std::optional<std::string_view> value,
                                                          std::optional<uint64_t> alignment,
                                                          bool complete) {
        auto baseType = type.baseType();
        string arrayName{ name.data(), name.length() };

        if (needDecorate()) {
            baseType = NameDecorator::decorate(baseType);
            arrayName = NameDecorator::decorate(arrayName);
        }

        ss << TAB_N();
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
            ss << TAB_N() << NameDecorator::decorate(name) << " = " << value << SCNL;
        } else {
            ss << TAB_N() << name << " = " << value << SCNL;
        }
        return ss;
    }

    Printer::Stream &Printer::strTabIf(bool needTabs) {
        ss << (needTabs ? TAB_N() : "");
        return ss;
    }

    Printer::Stream &Printer::strFunctionDecl(
        const string &returnType,
        const string &functionName,
        const std::vector<types::Type> &paramTypes,
        const std::vector<string> &paramValues,
        const string &end,
        const std::vector<string> &modifiers,
        const tests::Tests::MethodDescription::FPointerMap &fullDeclSubstitutions,
        bool isVariadic) {
        ss << TAB_N();
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
                                              const string &end,
                                              const std::vector<string> &modifiers) {
        return strFunctionDecl(method.returnType.usedType(), method.name, method.getParamTypes(),
                               method.getParamNames(), end, modifiers, method.functionPointers,
                               method.isVariadic);
    }


    Printer::Stream &
    Printer::strFunctionDeclWithParamString(const tests::Tests::MethodDescription &method,
                                            const string &end,
                                            const std::vector<string> &modifiers) {
        ss << TAB_N();
        for (const auto &modifier : modifiers) {
            ss << modifier << " ";
        }
        ss << method.returnType.usedType() << " " << method.name << "(" << method.paramsString
           << ")" << end;
        return ss;
    }

    Printer::Stream &Printer::strFunctionCall(std::string_view functionName,
                                              const std::vector<string> &args,
                                              const string &end,
                                              const std::optional<string> &classObj,
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
        string methodName(functionName);
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
                                              const string &end,
                                              bool needTabs) {
        strTabIf(needTabs);
        vector<string> parameters;
        for (const auto &param : method.params) {
            string maybeAmpersand = param.type.maybeJustPointer() ? "&" : "";
            parameters.push_back(maybeAmpersand + param.name);
        }
        auto classObjName = method.getClassName();
        return strFunctionCall(method.name, parameters, end, classObjName, needTabs, returnPointers);
    }

    Printer::Stream &Printer::strComment(const string &comment) {
        ss << TAB_N() << "// " << comment << NL;
        return ss;
    }

    Printer::Stream &Printer::commentBlockSeparator() {
        ss << TAB_N() << "//////////////////////////////////////////// " << NL;
        return ss;
    }

    string Printer::constrIndex(const string &arrayName, const string &ind) {
        return arrayName + "[" + ind + "]";
    }

    string Printer::constrIndex(const string &arrayName, int ind) {
        return constrIndex(arrayName, std::to_string(ind));
    }

    string Printer::constrMultiIndex(const string &arrayName, const std::vector<string> &indexes) {
        std::string element = arrayName;
        for (const auto &index : indexes) {
            element = constrIndex(element, index);
        }
        return element;
    }

    string Printer::constrMultiIndex(const string &arrayName, const std::vector<size_t> &indexes) {
        std::vector<string> strIndexes;
        for (size_t index : indexes) {
            strIndexes.push_back(std::to_string(index));
        }
        return constrMultiIndex(arrayName, strIndexes);
    }

    string Printer::constrMultiIndex(const std::vector<string> &indexes) {
        return constrMultiIndex("", indexes);
    }

    string Printer::constrFunctionCall(const tests::Tests::MethodDescription &method,
                                       size_t returnPointers,
                                       const string &end,
                                       bool needTabs) {
        std::stringstream func_ss;
        ss.swap(func_ss);
        strFunctionCall(method, returnPointers, end, needTabs);
        ss.swap(func_ss);
        return func_ss.str();
    }

    string Printer::constrFunctionCall(const string &functionName,
                                       const std::vector<string> &args,
                                       const string &end,
                                       const std::optional<string> &classObjName,
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
        ss << TAB_N() << str << SCNL;
        return ss;
    }
    string Printer::recursiveIteratorName(const string &prefix) const {
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
        ss << TAB_N();
        if (value.empty()) {
            ss << "return";
        } else {
            ss << "return " << value;
        }
        ss << SCNL;
        return ss;
    }

    std::stringstream& Printer::checkOverflowStubArray(const string &cntCall) {
        ss << TAB_N() << "if (" << cntCall << " == " <<
           types::TypesHandler::getElementsNumberInPointerOneDim(types::PointerUsage::PARAMETER) << ") {" << NL;
        tabsDepth++;
        ss << TAB_N() << cntCall << "--;" << NL;
        ss << RB();
        return ss;
    }


    std::stringstream &Printer::strStubForMethod(const Tests::MethodDescription &method,
                                                 const types::TypesHandler &typesHandler,
                                                 const string &prefix,
                                                 const string &suffix,
                                                 const string& methodName,
                                                 const string& nameForStub,
                                                 bool makeStatic) {
        auto methodCopy = method;
        methodCopy.name = method.name;

        string stubSymbolicVarName = getStubSymbolicVarName(nameForStub);
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
        vector<string> modifiers;
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

        string firstTimeCallVar = "firstTimeCall";
        strDeclareVar("static int", firstTimeCallVar, "1");
        const string cntCall = "cntCall";
        strDeclareVar("static int", cntCall, "0");
        ss << TAB_N() << "#ifdef " << PrinterUtils::KLEE_MODE << NL;
        tabsDepth++;
        ss << TAB_N() << "if (" << firstTimeCallVar << " == 1)" << LB();
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
        ss << TAB_N() << "#endif" << NL;

        checkOverflowStubArray(cntCall);

        returnValue = stubSymbolicVarName + "[" + cntCall + "++]";
        strReturn(returnValue) << RB() << NL;
        return ss;
    }

    string Printer::getStubSymbolicVarName(const string& methodName) {
        return methodName + PrinterUtils::KLEE_SYMBOLIC_SUFFIX;
    }

    Printer::Stream Printer::strKleeMakeSymbolic(const string &varName, bool needAmpersand, SRef pseudoName) {
        auto pointer = (needAmpersand ? "&" : "") + varName;
        auto size = "sizeof(" + varName + ")";
        auto name = "\"" + pseudoName + "\"";
        strFunctionCall("klee_make_symbolic", { pointer, size, name });
        return ss;
    }

    std::stringstream &Printer::strDeclareArrayOfFunctionPointerVar(
        const string &arrayType, const string &arrayName, const string &stubFunctionName) {
        size_t size =
            types::TypesHandler::getElementsNumberInPointerOneDim(types::PointerUsage::PARAMETER);
        strDeclareVar(arrayType, arrayName + "[" + std::to_string(size) + "]");
        strForBound("i", size) << " " << BNL;
        tabsDepth++;
        strAssignVar(arrayName + "[i]", stubFunctionName);
        tabsDepth--;
        ss << TAB_N() << "}" << NL;
        return ss;
    }

    std::stringstream &Printer::strTypedefFunctionPointer(const types::FunctionInfo &method,
                                                          const string &name) {
        auto paramTypes =
            CollectionUtils::transform(method.params, [](const auto &param) { return param.type; });
        ss << TAB_N() << "typedef ";
        strFunctionDecl(method.returnType.usedType(), StringUtils::stringFormat("(*%s)", name), paramTypes,
                        std::vector<string>(paramTypes.size(), ""), "") << SCNL;
        if (method.isArray) {
            ss << TAB_N() << "typedef ";
            strFunctionDecl(method.returnType.usedType(), StringUtils::stringFormat("(**%s)", name + "_arr"), paramTypes,
                            std::vector<string>(paramTypes.size(), ""), "") << SCNL;
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

        size_t pointerSize = types::TypesHandler::getElementsNumberInPointerMultiDim();
        auto typeObject = types::TypesHandler::isVoid(param.type.baseTypeObj())
                              ? types::Type::minimalScalarPointerType(2)
                              : param.type;
        auto baseType = typeObject.baseType();
        auto type = stringFormat("%s%s **", getConstQualifier(typeObject), baseType);
        string value =
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
        string destArg = stringFormat("(void *) %s%.*s", (needDereference ? "&"s : ""s),
                                      dest.length(), dest.data());
        string count = stringFormat("%s(%.*s)", SIZEOF, src.length(), src.data());
        strFunctionCall(MEMCPY, { destArg, std::string(src), count });
        return ss;
    }

    void printer::Printer::writeStubsForFunctionParams(const types::TypesHandler *typesHandler,
                                                       const Tests::MethodDescription &testMethod,
                                                       bool forKlee) {
        string scopeName = (forKlee ? testMethod.getClassName().value_or("") : "");
        string prefix = PrinterUtils::getKleePrefix(forKlee);
        for (const auto &[name, pointerFunctionStub] : testMethod.functionPointers) {
            string stubName = PrinterUtils::getFunctionPointerStubName(scopeName,
                                                                       testMethod.name, name, true);
            writeStubForParam(typesHandler, pointerFunctionStub, testMethod.name, stubName, true,
                              forKlee);
        }
    }

    void printer::Printer::writeExternForSymbolicStubs(const Tests::MethodDescription& testMethod) {
        std::unordered_map<string, string> symbolicNamesToTypesMap;
        for (const auto& testCase: testMethod.testCases) {
            for (size_t i = 0; i < testCase.stubValues.size(); i++) {
                symbolicNamesToTypesMap[testCase.stubValues[i].name] = testCase.stubValuesTypes[i].type.usedType();
            }
        }
        for (const auto& [name, type]: symbolicNamesToTypesMap) {
            strDeclareVar("extern \"C\" " + type, name);
        }
        ss << NL;
    }

    void printer::Printer::writeStubForParam(const types::TypesHandler *typesHandler,
                                             const std::shared_ptr<types::FunctionInfo> &fInfo,
                                             const string &methodName,
                                             const string &stubName,
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
        if (!checkedOnPrivate.count(type.getId()) && typesHandler->isStruct(type)) {
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

    void Printer::genStubForStructFunctionPointer(const string &structName,
                                                  const string &fieldName,
                                                  const string &stubName) {
        string name = PrinterUtils::getFieldAccess(structName, fieldName);
        strAssignVar(name, stubName);
    }

    void Printer::genStubForStructFunctionPointerArray(const string &structName,
                                                       const string &fieldName,
                                                       const string &stubName) {
        size_t size =
            types::TypesHandler::getElementsNumberInPointerOneDim(types::PointerUsage::PARAMETER);
        strForBound("i", size) << " " << BNL;
        tabsDepth++;
        string name = structName + "." + fieldName + "[i]";
        strAssignVar(name, stubName);
        tabsDepth--;
        ss << TAB_N() << "}" << NL;
    }

    void Printer::writeStubsForStructureFields(const Tests &tests) {
        ss << tests.stubs << NL;
    }

    utbot::Language Printer::getLanguage() const {
        return srcLanguage;
    }

    std::string Printer::getConstQualifier(const types::Type& type) {
        string constQualifier;
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
}
