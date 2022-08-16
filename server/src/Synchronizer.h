#ifndef UNITTESTBOT_SYNCHRONIZER_H
#define UNITTESTBOT_SYNCHRONIZER_H

#include "ProjectContext.h"

#include "stubs/StubGen.h"
#include "types/Types.h"

class StubOperator {
public:
    StubOperator(fs::path sourceFilePath, bool isHeader);
    explicit StubOperator(fs::path stubFilePath);
    bool operator ==(const StubOperator &other) const;
    [[nodiscard]] fs::path getStubPath(const utbot::ProjectContext &projectContext) const;
    [[nodiscard]] fs::path getSourceFilePath() const;
    [[nodiscard]] bool isHeader() const;
private:
    fs::path sourceFilePath;
    bool header;
};

class Synchronizer {
    BaseTestGen *const testGen;
    StubGen const *const stubGen;
    types::TypesHandler::SizeContext *sizeContext;

    [[nodiscard]] CollectionUtils::FileSet getOutdatedSourcePaths() const;

    [[nodiscard]] std::unordered_set<StubOperator, HashUtils::StubHash> getOutdatedStubs() const;

    bool isProbablyOutdated(const fs::path &srcFilePath) const;

    bool removeStubIfSourceAbsent(const StubOperator &stub) const;

    void synchronizeStubs(std::unordered_set<StubOperator, HashUtils::StubHash> &outdatedStubs,
                          const types::TypesHandler &typesHandler);
    void synchronizeWrappers(const CollectionUtils::FileSet &outdatedSourcePaths) const;

    std::shared_ptr<CompilationDatabase>
    createStubsCompilationDatabase(
        std::unordered_set<StubOperator, HashUtils::StubHash> &stubFiles,
        const fs::path &ccJsonStubDirPath) const;

    void prepareDirectory(fs::path const& stubDirectory);

    static std::unordered_set<StubOperator, HashUtils::StubHash>

    getStubSetFromSources(const CollectionUtils::FileSet &paths);
public:
    static std::unordered_set<StubOperator, HashUtils::StubHash>

    dropHeaders(const std::unordered_set<StubOperator, HashUtils::StubHash> &stubs);

    static CollectionUtils::FileSet dropHeaders(const CollectionUtils::FileSet &files);

    Synchronizer(BaseTestGen *testGen, StubGen const *stubGen, types::TypesHandler::SizeContext *sizeContext);

    void synchronize(const types::TypesHandler &typesHandler);

    [[nodiscard]] const CollectionUtils::FileSet &getSourceFiles() const;
    [[nodiscard]] std::unordered_set<StubOperator, HashUtils::StubHash> getStubsFiles() const;
};


#endif // UNITTESTBOT_SYNCHRONIZER_H
