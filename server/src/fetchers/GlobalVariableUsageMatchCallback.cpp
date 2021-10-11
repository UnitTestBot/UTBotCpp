/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "GlobalVariableUsageMatchCallback.h"

#include "clang-utils/AlignmentFetcher.h"
#include "clang-utils/ClangUtils.h"
#include "clang-utils/Matchers.h"

#include "loguru.h"

#include <utility>

void GlobalVariableUsageMatchCallback::run(const MatchFinder::MatchResult &Result) {
    ExecUtils::throwIfCancelled();
    checkUsage(Result);
}

static std::unordered_set<std::string> BLACK_LIST = { "stdin", "stdout", "stderr" };

void GlobalVariableUsageMatchCallback::checkUsage(const MatchFinder::MatchResult &Result) {
    if (const auto *pVarDecl =
            Result.Nodes.getNodeAs<clang::VarDecl>(Matchers::GLOBAL_VARIABLE_USAGE)) {
        clang::QualType varType = pVarDecl->getType();
        string name = pVarDecl->getNameAsString();
        if (!pVarDecl->isKnownToBeDefined()) {
            LOG_S(DEBUG) << "Variable \"" << name << "\" was skipped - it has no definition.";
            return;
        }
        if (const auto *pFunctionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>(
                Matchers::FUNCTION_USED_GLOBAL_VARIABLE)) {
            if (varType.isConstant(pVarDecl->getASTContext())) {
                LOG_S(MAX) << "Variable \"" << name << "\" was skipped - it is being constant.";
                return;
            }
            if (CollectionUtils::contains(BLACK_LIST, name)) {
                LOG_S(MAX) << "Variable \"" << name << "\" was skipped - it is being blacklisted.";
                return;
            }
            if (ClangUtils::isIncomplete(pVarDecl->getType())) {
                LOG_S(MAX) << "Variable \"" << name
                           << "\" was skipped - it's type has no definition in current "
                              "translation unit.";
                return;
            }
            handleUsage(pFunctionDecl, pVarDecl);
        }
    }
}

void GlobalVariableUsageMatchCallback::handleUsage(const clang::FunctionDecl *functionDecl,
                                                   const clang::VarDecl *varDecl) {
    clang::SourceManager &sourceManager = functionDecl->getASTContext().getSourceManager();
    fs::path sourceFilePath =
        sourceManager.getFileEntryForID(sourceManager.getMainFileID())->tryGetRealPathName().str();
    auto const &[iterator, inserted] =
        usages.emplace(varDecl->getNameAsString(), functionDecl->getNameAsString());
    auto const &usage = *iterator;

    LOG_S(MAX) << "Found usage of global variable \'" << usage.variableName << "\' in function \'"
               << usage.functionName << "\'";

    if (!inserted) {
        LOG_S(MAX) << "Skip it, as it has been already occurred";
        return;
    }

    auto &methods = (*parent->projectTests).at(sourceFilePath).methods;
    auto &method = methods[usage.functionName];
    const clang::QualType realParamType = varDecl->getType().getCanonicalType();
    const std::string usedParamTypeString = varDecl->getType().getAsString();
    types::Type paramType(realParamType, usedParamTypeString);
    method.globalParams.emplace_back(paramType, usage.variableName, AlignmentFetcher::fetch(varDecl));
}

GlobalVariableUsageMatchCallback::Usage::Usage(string variableName, string functionName)
    : variableName(std::move(variableName)), functionName(std::move(functionName)) {
}

bool GlobalVariableUsageMatchCallback::Usage::operator==(
    const GlobalVariableUsageMatchCallback::Usage &rhs) const {
    return variableName == rhs.variableName && functionName == rhs.functionName;
}

bool GlobalVariableUsageMatchCallback::Usage::operator!=(
    const GlobalVariableUsageMatchCallback::Usage &rhs) const {
    return !(rhs == *this);
}

std::size_t GlobalVariableUsageMatchCallback::UsageHash::operator()(
    const GlobalVariableUsageMatchCallback::Usage &usage) const {
    size_t seed = 0;
    HashUtils::hashCombine(seed, usage.variableName, usage.functionName);
    return seed;
}