#include "ReturnTypesFetcher.h"

#include "fetchers/Fetcher.h"
#include "testgens/BaseTestGen.h"

void ReturnTypesFetcher::fetch(const ProgressWriter *progressWriter) {
    const std::shared_ptr<CompilationDatabase> buildDatabases[] = {
            testGen->getTargetBuildDatabase()->compilationDatabase,
            testGen->getProjectBuildDatabase()->compilationDatabase};
    for (const auto &buildDatabase: buildDatabases) {
        tests::TestsMap testsMap;
        for (const auto &filePath: buildDatabase->getAllFiles()) {
            testsMap[filePath];
        }
        Fetcher(Fetcher::Options::Value::RETURN_TYPE_NAMES_ONLY,
                buildDatabase, testsMap, nullptr, nullptr,
                testGen->compileCommandsJsonPath, false)
                .fetchWithProgress(progressWriter, "Fetching return types for functions", true);
        for (auto const &[sourceFilePath, test]: testsMap) {
            for (const auto &[methodName, methodDescription]: test.methods) {
                if (testGen->methodNameToReturnTypeMap.count(methodName)) {
                    continue;
                }
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
}
