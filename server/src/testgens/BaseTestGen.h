#ifndef UNITTESTBOT_BASETESTGEN_H
#define UNITTESTBOT_BASETESTGEN_H

#include "ProjectContext.h"
#include "SettingsContext.h"
#include "Tests.h"
#include "building/ProjectBuildDatabase.h"
#include "building/TargetBuildDatabase.h"
#include "printers/TestsPrinter.h"
#include "streams/tests/TestsWriter.h"
#include "stubs/Stubs.h"
#include "types/Types.h"

#include <grpcpp/grpcpp.h>
#include <protobuf/testgen.grpc.pb.h>

#include "utils/path/FileSystemPath.h"

class BaseTestGen {
public:
    const utbot::ProjectContext projectContext;
    const utbot::SettingsContext settingsContext;
    ProgressWriter *const progressWriter;
    fs::path serverBuildDir;

    fs::path compileCommandsJsonPath;

    CollectionUtils::FileSet sourcePaths, testingMethodsSourcePaths;
    tests::TestsMap tests;
    std::unordered_map<std::string, types::Type> methodNameToReturnTypeMap;
    std::vector<Stubs> synchronizedStubs;
    types::TypeMaps types;

    CollectionUtils::FileSet targetSources;

    virtual std::string toString() = 0;

    bool needToBeMocked() const;

    bool isBatched() const;

    void setTargetPath(fs::path _targetPath);

    virtual ~BaseTestGen() = default;

    std::shared_ptr<const ProjectBuildDatabase> getProjectBuildDatabase() const;

    std::shared_ptr<const TargetBuildDatabase> getTargetBuildDatabase() const;

    std::shared_ptr<ProjectBuildDatabase> getProjectBuildDatabase();

    std::shared_ptr<TargetBuildDatabase> getTargetBuildDatabase();

    const CollectionUtils::FileSet &getTargetSourceFiles() const;

    const CollectionUtils::FileSet &getProjectSourceFiles() const;

    std::shared_ptr<const BuildDatabase::ObjectFileInfo>
    getClientCompilationUnitInfo(const fs::path &path, bool fullProject = false) const;

protected:
    std::shared_ptr<ProjectBuildDatabase> projectBuildDatabase;
    std::shared_ptr<TargetBuildDatabase> targetBuildDatabase;

    BaseTestGen(const testsgen::ProjectContext &projectContext,
                const testsgen::SettingsContext &settingsContext,
                ProgressWriter *progressWriter,
                bool testMode);

    void setInitializedTestsMap();

    virtual void setTargetForSource(fs::path const &sourcePath) = 0;

    void updateTargetSources(fs::path _targetPath);
};


#endif // UNITTESTBOT_BASETESTGEN_H
