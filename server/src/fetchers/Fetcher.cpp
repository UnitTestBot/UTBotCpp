/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Fetcher.h"

#include "ArraySubscriptFetcherMatchCallback.h"
#include "FunctionDeclsMatchCallback.h"
#include "GlobalVariableUsageMatchCallback.h"
#include "IncludeFetchSourceFileCallback.h"
#include "Paths.h"
#include "ReturnStmtFetcherMatchCallback.h"
#include "SingleFileParseModeCallback.h"
#include "TypeDeclsMatchCallback.h"
#include "clang-utils/SourceToHeaderMatchCallback.h"

#include "loguru.hpp"

#include "utils/path/FileSystemPath.h"
#include <memory>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;
using namespace Matchers;

Fetcher::Fetcher(Options options,
                 const shared_ptr<CompilationDatabase> &compilationDatabase,
                 tests::TestsMap &tests,
                 types::TypeMaps *types,
                 uint64_t *pointerSize,
                 uint64_t *maximumAlignment,
                 const fs::path &compileCommandsJsonPath,
                 bool fetchFunctionBodies)
    : options(options), projectTests(&tests), projectTypes(types),
      pointerSize(pointerSize), maximumAlignment(maximumAlignment),
      fetchFunctionBodies(fetchFunctionBodies), clangToolRunner(compilationDatabase) {
    buildRootPath = Paths::subtractPath(compileCommandsJsonPath.string(),
                                        CompilationUtils::MOUNTED_CC_JSON_DIR_NAME);
    if (options.has(Options::Value::TYPE)) {
        addMatcher<TypeDeclsMatchCallback>(anyTypeDeclarationMatcher);
        addMatcher<TypeDeclsMatchCallback>(structJustDeclMatcher);
    }
    if (options.has(Options::Value::FUNCTION)) {
        addMatcher<FunctionDeclsMatchCallback>(functionDefinitionMatcher, false, false, false);
    }
    if (options.has(Options::Value::GLOBAL_VARIABLE_USAGE)) {
        addMatcher<GlobalVariableUsageMatchCallback>(globalVariableUsageMatcher);
    }
    if (options.has(Options::Value::ARRAY_USAGE)) {
        addMatcher<ArraySubscriptFetcherMatchCallback>(arraySubscriptMatcher);
        addMatcher<ReturnStmtFetcherMatchCallback>(returnMatcher);
    }
    if (options.has(Options::Value::INCLUDE)) {
        auto callback = std::make_unique<IncludeFetchSourceFileCallback>(this);
        sourceFileCallbacks.add(std::move(callback));
    }
    if (options.has(Options::Value::FUNCTION_NAMES_ONLY)) {
        addMatcher<FunctionDeclsMatchCallback>(functionDefinitionMatcher, true, false, false);
        auto callback = std::make_unique<SingleFileParseModeCallback>();
        sourceFileCallbacks.add(std::move(callback));
    }
    if (options.has(Options::Value::RETURN_TYPE_NAMES_ONLY)) {
        addMatcher<FunctionDeclsMatchCallback>(functionDefinitionMatcher, false, true, true);
        auto callback = std::make_unique<SingleFileParseModeCallback>();
        sourceFileCallbacks.add(std::move(callback));
    }
}

void Fetcher::fetch() {
    LOG_SCOPE_FUNCTION(DEBUG);
    auto factory = newFrontendActionFactory(&finder, &sourceFileCallbacks);
    clangToolRunner.run(projectTests, factory.get());

    postProcess();
}

void Fetcher::fetchWithProgress(const ProgressWriter *progressWriter,
                                std::string const &message,
                                bool ignoreDiagnostics) {
    LOG_SCOPE_FUNCTION(DEBUG);
    auto factory = newFrontendActionFactory(&finder, &sourceFileCallbacks);
    clangToolRunner.runWithProgress(projectTests, factory.get(), progressWriter,
                                    message, ignoreDiagnostics);

    postProcess();
}

void Fetcher::postProcess() const {
    if (options.has(Options::Value::FUNCTION) && maximumAlignment != nullptr) {
        for (auto projectTestsIterator = projectTests->begin();
             projectTestsIterator != projectTests->end(); projectTestsIterator++) {
            tests::Tests &tests = projectTestsIterator.value();
            for (auto it = tests.methods.begin(); it != tests.methods.end(); it++) {
                auto &methodDescription = it.value();
                for (auto &param : methodDescription.params) {
                    if (param.alignment.has_value()) {
                        if (param.alignment.value() == 0) {
                            param.alignment = *maximumAlignment;
                        }
                    }
                }
            }
        }
    }
    if (options.has(Options::Value::ARRAY_USAGE)) {
        for (auto projectTestsIterator = projectTests->begin();
             projectTestsIterator != projectTests->end(); projectTestsIterator++) {
            tests::Tests &tests = projectTestsIterator.value();
            for (const auto &methodName : CollectionUtils::getKeys(tests.methods)) {
                auto &methodDescription = tests.methods[methodName];
                for (auto &param : methodDescription.params) {
                    if (types::TypesHandler::isCStringType(param.type)) {
                        // Char pointer is most likely to be a string
                        param.type.maybeArray = true;
                    }
                }
                for (auto &param : methodDescription.globalParams) {
                    // TODO: handle globals conveniently
                    param.type.maybeArray = true;
                }
            }
        }
    }
}

Fetcher::Options::Options(Fetcher::Options::Value value) : value(value) {
}
bool Fetcher::Options::has(Fetcher::Options::Value other) const {
    return static_cast<int>(value & other) != 0;
}

std::shared_ptr<Fetcher::FileToStringSet> Fetcher::getStructsToDeclare() const {
    return structsToDeclare;
}