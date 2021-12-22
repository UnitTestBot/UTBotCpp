/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_LINKER_H
#define UNITTESTBOT_LINKER_H

#include "BuildResult.h"
#include "IRParser.h"
#include "KleeGenerator.h"
#include "RunCommand.h"
#include "printers/DefaultMakefilePrinter.h"
#include "printers/NativeMakefilePrinter.h"
#include "testgens/BaseTestGen.h"
#include "utils/CollectionUtils.h"
#include "utils/MakefileUtils.h"
#include "utils/Void.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stubs/StubGen.h>

class Linker {
public:
    Linker(BaseTestGen &testGen,
           StubGen stubGen,
           shared_ptr<LineInfo> lineInfo,
           shared_ptr<KleeGenerator> kleeGenerator);

    void prepareArtifacts();

    void writeMakefiles();

    std::vector<tests::TestMethod> getTestMethods();

    BuildResult
    addLinkTargetRecursively(const fs::path &fileToBuild,
                             printer::DefaultMakefilePrinter &bitcodeLinkMakefilePrinter,
                             const CollectionUtils::FileSet &stubSources,
                             const CollectionUtils::MapFileTo<fs::path> &bitcodeFiles,
                             std::string const &suffixForParentOfStubs,
                             bool hasParent,
                             const std::optional<fs::path> &testedFilePath);

    struct LinkResult {
        fs::path bitcodeOutput;
        CollectionUtils::FileSet stubsSet;
        CollectionUtils::FileSet presentedFiles;
    };
private:
    BaseTestGen &testGen;
    shared_ptr<KleeGenerator> kleeGenerator;
    StubGen stubGen;
    shared_ptr<LineInfo> lineInfo;

    CollectionUtils::FileSet testedFiles;
    CollectionUtils::MapFileTo<fs::path> bitcodeFileName;
    CollectionUtils::FileSet brokenLinkFiles;

    CollectionUtils::MapFileTo<std::string> linkMakefiles;

    IRParser irParser;

    fs::path getSourceFilePath();

    bool isForOneFile();

    std::vector<fs::path> getTargetList(const fs::path &sourceFile, const fs::path &objectFile) const;

    Result<Linker::LinkResult> linkForTarget(const fs::path &target, const fs::path &sourceFilePath,
                                             const std::shared_ptr<const BuildDatabase::ObjectFileInfo> &compilationUnitInfo,
                                             const fs::path &objectFile);

    Result<Linker::LinkResult> linkWholeTarget(const fs::path &target);
    void linkForOneFile(const fs::path &sourceFilePath);
    void linkForProject();
    Result<Linker::LinkResult> link(const CollectionUtils::MapFileTo<fs::path> &bitcodeFiles,
                                    const fs::path &root,
                                    std::string const &suffixForParentOfStubs,
                                    const std::optional<fs::path> &testedFilePath,
                                    const CollectionUtils::FileSet &stubSources,
                                    bool errorOnMissingBitcode = true);

    void checkSiblingsExist(const CollectionUtils::FileSet &archivedFiles) const;
    void addToGenerated(const CollectionUtils::FileSet &objectFiles, const fs::path &output);
    fs::path getPrefixPath(const vector<fs::path> &dependencies, fs::path defaultPath) const;

    Result<CollectionUtils::FileSet> generateStubsMakefile(const fs::path &root,
                                                           const fs::path &outputFile,
                                                           const fs::path &stubsMakefile) const;
    Result<utbot::Void> linkWithStubsIfNeeded(const fs::path &linkMakefile, const fs::path &targetBitcode) const;

    void declareRootLibraryTarget(printer::DefaultMakefilePrinter &bitcodeLinkMakefilePrinter,
                                  const fs::path &output,
                                  const vector<fs::path> &bitcodeDependencies,
                                  const fs::path &prefixPath,
                                  const utbot::RunCommand &removeAction,
                                  vector<utbot::LinkCommand> archiveActions);

    string getLinkArgument(const string &argument,
                           const fs::path &workingDir,
                           const CollectionUtils::MapFileTo<fs::path> &dependencies,
                           const BuildDatabase::TargetInfo &linkUnitInfo,
                           const fs::path &output);
    vector<utbot::LinkCommand>
    getLinkActionsForExecutable(const fs::path &workingDir,
                                const CollectionUtils::MapFileTo<fs::path> &dependencies,
                                const BuildDatabase::TargetInfo &linkUnitInfo,
                                const fs::path &output);
};


#endif //UNITTESTBOT_LINKER_H
