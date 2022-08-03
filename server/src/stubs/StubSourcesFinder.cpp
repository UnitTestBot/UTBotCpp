#include "StubSourcesFinder.h"

#include "loguru.h"

StubSourcesFinder::StubSourcesFinder(std::shared_ptr<BuildDatabase> buildDatabase)
    : buildDatabase(std::move(buildDatabase)) {
}

std::vector<fs::path> StubSourcesFinder::find(const fs::path& testedFilePath) {
    CollectionUtils::FileSet libraryBitcodeFiles = getLibraryBitcodeFiles(testedFilePath);
    std::vector<fs::path> stubSources;
    stubSources.reserve(libraryBitcodeFiles.size());
    for (const auto &bitcodeFile : libraryBitcodeFiles) {
        fs::path sourcePath =
            buildDatabase->getClientCompilationUnitInfo(bitcodeFile)->getSourcePath();
        stubSources.emplace_back(std::move(sourcePath));
    }
    return stubSources;
}

std::vector<fs::path> StubSourcesFinder::excludeFind(const fs::path &testedFilePath,
                                                     const fs::path &rootPath) {
    auto allBitcodeFiles = buildDatabase->getArchiveObjectFiles(rootPath);
    CollectionUtils::FileSet libraryBitcodeFiles = getLibraryBitcodeFiles(testedFilePath);
    std::vector<fs::path> stubSources;
    for (const auto &bitcodeFile : allBitcodeFiles) {
        if (CollectionUtils::contains(libraryBitcodeFiles, bitcodeFile))
            continue;
        fs::path sourcePath =
            buildDatabase->getClientCompilationUnitInfo(bitcodeFile)->getSourcePath();
        stubSources.emplace_back(std::move(sourcePath));
    }
    return stubSources;
}

CollectionUtils::FileSet StubSourcesFinder::getLibraryBitcodeFiles(const fs::path& testedFilePath) {
    fs::path linkUnit = buildDatabase->getClientCompilationUnitInfo(testedFilePath)->linkUnit;
    return buildDatabase->getArchiveObjectFiles(linkUnit);
}

void StubSourcesFinder::printAllModules() {
    CollectionUtils::MapFileTo<std::vector<fs::path>> modules;
    for (const auto &compileCommand : buildDatabase->getAllCompileCommands()) {
        modules[compileCommand->linkUnit].emplace_back(compileCommand->getSourcePath());
    }
    std::stringstream messageBuilder;
    messageBuilder << "Printing all modules content...\n";
    for (const auto&[module, files]: modules) {
        messageBuilder << module.filename().stem().string() << "\n";
        for (const auto& file: files) {
            messageBuilder << "\t" << file.filename() << "\n";
        }
    }
    LOG_S(DEBUG) << messageBuilder.str();
}
