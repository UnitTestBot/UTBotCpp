/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "CCJsonPrinter.h"

#include "Paths.h"
#include "utils/SanitizerUtils.h"
#include "utils/JsonUtils.h"
#include "environment/EnvironmentPaths.h"

using printer::CCJsonPrinter;

const string CCJsonPrinter::DEFAULT_BUILD_FLAGS = " -c -g -O0";

void printer::CCJsonPrinter::createDummyBuildDB(const CollectionUtils::FileSet &filePaths,
                                                const fs::path &tmpDirPath) {
    fs::create_directories(tmpDirPath);
    json compileJson = json::array(), linkJson = json::array();
    for (const fs::path &filePath : filePaths) {
        fs::path sourceFile = filePath;
        fs::path objectFile = Paths::replaceExtension(sourceFile, ".o");
        std::string compiler = CompilationUtils::getDefaultCompilerForSourceFile(filePath);
        std::vector<std::string> compileArguments = { compiler };
        auto buildFlags = StringUtils::split(DEFAULT_BUILD_FLAGS);
        CollectionUtils::extend(compileArguments, buildFlags);
        std::vector<std::string> compileFlags = { "-o", objectFile.string(), filePath };
        CollectionUtils::extend(compileArguments, compileFlags);
        auto compileUnit = getUnit(compileArguments, tmpDirPath, { filePath }, false);

        auto executableUnit =
            getUnit({ compiler, "-o", tmpDirPath / "executable", objectFile.string() }, tmpDirPath,
                    { objectFile }, true);
        compileJson.push_back(compileUnit);
        linkJson.push_back(executableUnit);
    }
    JsonUtils::writeJsonToFile(tmpDirPath / "compile_commands.json", compileJson);
    JsonUtils::writeJsonToFile(tmpDirPath / "link_commands.json", linkJson);
}

void printer::CCJsonPrinter::createCDb(const vector<utbot::CompileCommand> &compileCommands,
                                       const fs::path &tmpDirPath) {
    fs::create_directories(tmpDirPath);
    json compileJson = json::array();
    for (const auto & compileCommand : compileCommands) {
        auto commandLine = compileCommand.getCommandLine();
        auto compileUnit = getUnit({ commandLine.begin(), commandLine.end() }, tmpDirPath,
                                   { compileCommand.getSourcePath() }, false);
        compileJson.push_back(compileUnit);
    }
    JsonUtils::writeJsonToFile(tmpDirPath / "compile_commands.json", compileJson);

    auto defaultCompiler =
        CompilationUtils::getDefaultCompilerForSourceFile(compileCommands[0].getSourcePath());
    auto compiler = compileCommands.empty() ? Paths::getUTBotClang().string() : defaultCompiler;
    vector<string> linkArgs = { compiler, "-o", tmpDirPath / "executable" };
    vector<fs::path> objectFiles;
    objectFiles.reserve(compileCommands.size());
    for (const auto &compileCommand : compileCommands) {
        fs::path path = Paths::replaceExtension(compileCommand.getSourcePath(), ".o");
        objectFiles.emplace_back(path);
    }
    CollectionUtils::extend(linkArgs, objectFiles);
    auto linkUnit = getUnit(linkArgs, tmpDirPath, objectFiles, true);
    json linkJson = json::array();
    linkJson.push_back(linkUnit);
    JsonUtils::writeJsonToFile(tmpDirPath / "link_commands.json", linkJson);
}

json printer::CCJsonPrinter::getUnit(const vector<string> &command,
                                     const fs::path &directory,
                                     const vector<fs::path> &sourceFiles,
                                     bool forLinkJson) {
    json j;
    j["directory"] = directory;
    j["arguments"] = json(command);
    if (forLinkJson) {
        j["files"] = json(sourceFiles);
    } else {
        j["file"] = sourceFiles[0];
    }
    return j;
}
