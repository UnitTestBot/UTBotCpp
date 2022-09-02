#ifndef UNITTESTBOT_LINKER_H
#define UNITTESTBOT_LINKER_H

#include "BuildResult.h"
#include "IRParser.h"
#include "KleeGenerator.h"
#include "RunCommand.h"
#include "printers/DefaultMakefilePrinter.h"
#include "printers/NativeMakefilePrinter.h"
#include "printers/TestMakefilesPrinter.h"
#include "testgens/BaseTestGen.h"
#include "utils/CollectionUtils.h"
#include "utils/MakefileUtils.h"
#include "utils/Void.h"
#include "stubs/StubGen.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Linker {
public:
    Linker(BaseTestGen &testGen,
           StubGen stubGen,
           std::shared_ptr<LineInfo> lineInfo,
           std::shared_ptr<KleeGenerator> kleeGenerator);

    void prepareArtifacts();

    std::vector<tests::TestMethod> getTestMethods();

    BuildResult
    addLinkTargetRecursively(const fs::path &fileToBuild,
                             printer::DefaultMakefilePrinter &bitcodeLinkMakefilePrinter,
                             const CollectionUtils::FileSet &stubSources,
                             const CollectionUtils::MapFileTo<fs::path> &bitcodeFiles,
                             std::string const &suffixForParentOfStubs,
                             bool hasParent,
                             const std::optional<fs::path> &testedFilePath,
                             bool shouldChangeDirectory = false);

    struct LinkResult {
        fs::path bitcodeOutput;
        CollectionUtils::FileSet stubsSet;
        CollectionUtils::FileSet presentedFiles;
    };
private:
    BaseTestGen &testGen;
    std::shared_ptr<KleeGenerator> kleeGenerator;
    StubGen stubGen;
    std::shared_ptr<LineInfo> lineInfo;

    CollectionUtils::FileSet testedFiles;
    CollectionUtils::MapFileTo<fs::path> bitcodeFileName;
    CollectionUtils::FileSet brokenLinkFiles;

    IRParser irParser;

    fs::path getSourceFilePath();

    bool isForOneFile();

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
    fs::path getPrefixPath(const std::vector<fs::path> &dependencies, fs::path defaultPath) const;

    Result<CollectionUtils::FileSet> generateStubsMakefile(const fs::path &root,
                                                           const fs::path &outputFile,
                                                           const fs::path &stubsMakefile) const;
    Result<utbot::Void> linkWithStubsIfNeeded(const fs::path &linkMakefile, const fs::path &targetBitcode) const;

    fs::path declareRootLibraryTarget(printer::DefaultMakefilePrinter &bitcodeLinkMakefilePrinter,
                                      const fs::path &output,
                                      const std::vector<fs::path> &bitcodeDependencies,
                                      const fs::path &prefixPath,
                                      std::vector<utbot::LinkCommand> archiveActions,
                                      bool shouldChangeDirectory = false);

    std::string getLinkArgument(const std::string &argument,
                                const fs::path &workingDir,
                                const CollectionUtils::MapFileTo<fs::path> &dependencies,
                                const BuildDatabase::TargetInfo &linkUnitInfo,
                                const fs::path &output);

    std::vector<utbot::LinkCommand>
    getLinkActionsForExecutable(const fs::path &workingDir,
                                const CollectionUtils::MapFileTo<fs::path> &dependencies,
                                const BuildDatabase::TargetInfo &linkUnitInfo,
                                const fs::path &output,
                                bool shouldChangeDirectory = true);
};


#endif //UNITTESTBOT_LINKER_H
