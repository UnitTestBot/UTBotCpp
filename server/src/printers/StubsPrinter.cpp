#include "StubsPrinter.h"

#include "Paths.h"
#include "utils/TimeUtils.h"

using printer::StubsPrinter;

printer::StubsPrinter::StubsPrinter(utbot::Language srcLanguage) : Printer(srcLanguage) {
}

Stubs printer::StubsPrinter::genStubFile(const tests::Tests &tests,
                                         const types::TypesHandler &typesHandler,
                                         const utbot::ProjectContext &projectContext) {
    resetStream();
    Stubs stubFile;
    stubFile.filePath = Paths::sourcePathToStubPath(projectContext, tests.sourceFilePath);
    long long creationTime = TimeUtils::convertFileToSystemClock(fs::file_time_type::clock::now())
                                 .time_since_epoch()
                                 .count();
    strComment(std::to_string(creationTime));
    strComment("Please, do not change the line above") << NL;
    writeCopyrightHeader();
    ss << "#ifdef " << PrinterUtils::KLEE_MODE << NL;
    ss << LINE_INDENT() << "extern void klee_make_symbolic(void *addr, unsigned long long nbytes, const char *name);" << NL;
    ss << "#endif" << NL;
    strInclude(Paths::sourcePathToHeaderInclude(tests.sourceFilePath));
    ss << NL;
    ss << "#pragma GCC visibility push (default)" << NL;
    strDefine(PrinterUtils::C_NULL, "((void*)0)") << NL;
    for (const auto &[_, method] : tests.methods) {
        auto methodCopy = method;
        auto returnMangledName = PrinterUtils::getReturnMangledName(methodCopy.name);
        if (CollectionUtils::containsKey(methodCopy.functionPointers, returnMangledName)) {
            strTypedefFunctionPointer(*methodCopy.functionPointers[returnMangledName],
                                            returnMangledName);
            types::Type copyReturnType = methodCopy.returnType;
            if (methodCopy.returnType.isArrayOfPointersToFunction()) {
                returnMangledName += "_arr";
            }
            copyReturnType.replaceUsedType(returnMangledName);
            methodCopy.returnType = copyReturnType;
        }
        for (auto& param : methodCopy.params) {
            auto paramMangledName = PrinterUtils::getParamMangledName(param.name, methodCopy.name);
            if (CollectionUtils::containsKey(methodCopy.functionPointers, param.name)) {
                strTypedefFunctionPointer(*methodCopy.functionPointers[param.name], paramMangledName);
                types::Type copyParamType = param.type;
                if (param.type.isArrayOfPointersToFunction()) {
                    paramMangledName += "_arr";
                }
                copyParamType.replaceUsedType(paramMangledName);
                param.type = copyParamType;
            }
        }

        if (!typesHandler.omitMakeSymbolic(methodCopy.returnType)) {
            std::string stubSymbolicVarName = StubsUtils::getStubSymbolicVarName(method.name);
            strDeclareArrayVar(types::Type::createArray(method.returnType), stubSymbolicVarName,
                               types::PointerUsage::PARAMETER);
        }
	
	if (methodCopy.sourceBody) {
            strFunctionDecl(methodCopy, " ");
            ss << methodCopy.sourceBody.value() << NL;
        } else {
            strStubForMethod(methodCopy, typesHandler, "", "", "", methodCopy.name);
        };
        ss << NL;
    }
    ss << "#pragma GCC visibility pop" << NL;
    stubFile.code = ss.str();
    return stubFile;
}
