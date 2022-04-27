/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_BASETESTGEN_H
#define UNITTESTBOT_BASETESTGEN_H

#include "ProjectContext.h"
#include "ProjectTarget.h"
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

using std::shared_ptr;
using std::vector;

class BaseTestGen {
public:
    const utbot::ProjectContext projectContext;
    const utbot::SettingsContext settingsContext;
    ProgressWriter *const progressWriter;
    fs::path serverBuildDir;

    fs::path compileCommandsJsonPath;
    shared_ptr<CompilationDatabase> compilationDatabase;
    shared_ptr<BuildDatabase> buildDatabase;

    CollectionUtils::FileSet sourcePaths, testingMethodsSourcePaths;
    tests::TestsMap tests;
    std::unordered_map<string, types::Type> methodNameToReturnTypeMap;
    vector<Stubs> synchronizedStubs;
    types::TypeMaps types;

    std::optional<fs::path> targetPath;
    CollectionUtils::FileSet targetSources;

    virtual string toString() = 0;

    bool needToBeMocked() const;

    bool isBatched() const;

    bool hasAutoTarget() const;
    fs::path const &getTargetPath() const;
    void setTargetPath(fs::path _targetPath);

    virtual ~BaseTestGen() = default;
protected:
    BaseTestGen(const testsgen::ProjectContext &projectContext,
                const testsgen::SettingsContext &settingsContext,
                ProgressWriter *progressWriter,
                bool testMode);

    void setInitializedTestsMap();

    virtual void setTargetForSource(fs::path const& sourcePath) = 0;

    void updateTargetSources();
};


#endif // UNITTESTBOT_BASETESTGEN_H
