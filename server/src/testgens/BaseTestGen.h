#ifndef UNITTESTBOT_BASETESTGEN_H
#define UNITTESTBOT_BASETESTGEN_H

#include "ProjectContext.h"
#include "SettingsContext.h"
#include "Tests.h"
#include "building/BuildDatabase.h"
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
    std::shared_ptr<CompilationDatabase> compilationDatabase;
    std::shared_ptr<BuildDatabase> baseBuildDatabase;
    std::shared_ptr<BuildDatabase> buildDatabase;

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
protected:
    BaseTestGen(const testsgen::ProjectContext &projectContext,
                const testsgen::SettingsContext &settingsContext,
                ProgressWriter *progressWriter,
                bool testMode);

    void setInitializedTestsMap();

    virtual void setTargetForSource(fs::path const& sourcePath) = 0;

    void updateTargetSources(fs::path _targetPath);
};


#endif // UNITTESTBOT_BASETESTGEN_H
