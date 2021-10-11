/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "ReturnTypesFetcher.h"

#include "fetchers/Fetcher.h"
#include "testgens/BaseTestGen.h"

void ReturnTypesFetcher::fetch(ProgressWriter *const progressWriter,
                               const CollectionUtils::FileSet &allFiles) {
    tests::TestsMap testsMap;
    for (const auto &filePath : allFiles) {
        testsMap[filePath];
    }
    Fetcher(Fetcher::Options::Value::RETURN_TYPE_NAMES_ONLY,
                 testGen->compilationDatabase, testsMap, nullptr, nullptr, nullptr,
                 testGen->compileCommandsJsonPath, false)
        .fetchWithProgress(progressWriter, "Fetching return types for functions", true);
    for (auto const &[sourceFilePath, test] : testsMap) {
        for (const auto& [methodName, methodDescription]: test.methods) {
            auto returnTypedefName = PrinterUtils::getReturnMangledName(methodDescription.name);
            if (CollectionUtils::containsKey(methodDescription.functionPointers, returnTypedefName)) {
                types::Type returnTypeCopy = methodDescription.returnType;
                returnTypeCopy.replaceUsedType(returnTypedefName);
                testGen->methodNameToReturnTypeMap[methodName] = returnTypeCopy;
            } else {
                testGen->methodNameToReturnTypeMap[methodName] = methodDescription.returnType;
            }
        }
    }
}
