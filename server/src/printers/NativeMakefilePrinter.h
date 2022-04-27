/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_NATIVEMAKEFILEPRINTER_H
#define UNITTESTBOT_NATIVEMAKEFILEPRINTER_H

#include "BuildResult.h"
#include "printers/DefaultMakefilePrinter.h"
#include "testgens/BaseTestGen.h"

#include "utils/path/FileSystemPath.h"
#include <vector>

namespace printer {
    static const std::string FORCE = ".FORCE";

    class NativeMakefilePrinter : public DefaultMakefilePrinter {
    private:
        const utbot::ProjectContext projectContext;
        shared_ptr<BuildDatabase> buildDatabase;
        fs::path rootPath;

        fs::path primaryCompiler;
        fs::path primaryCxxCompiler;
        CompilationUtils::CompilerName primaryCompilerName;
        CompilationUtils::CompilerName primaryCxxCompilerName;
        fs::path cxxLinker;

        string pthreadFlag;
        string coverageLinkFlags;
        string sanitizerLinkFlags;

        fs::path buildDirectory, dependencyDirectory;

        std::vector<fs::path> artifacts{};

        CollectionUtils::FileSet const *const stubSources = nullptr;
        CollectionUtils::MapFileTo<BuildResult> buildResults{};

        std::optional<fs::path> sharedOutput;

        fs::path getTemporaryDependencyFile(fs::path const &file);

        fs::path getDependencyFile(fs::path const &file);

        void gtestAllTargets(const utbot::CompileCommand &defaultCompileCommand,
                             const fs::path &gtestBuildDir);

        void gtestMainTargets(const utbot::CompileCommand &defaultCompileCommand,
                              const fs::path &gtestBuildDir);

        fs::path getSharedLibrary(const fs::path &filePath);

        void addTestTarget(const fs::path &sourcePath);

        BuildResult addObjectFile(const fs::path &objectFile,
                                  const std::string &suffixForParentOfStubs);

        void addCompileTarget(const fs::path &sourcePath,
                              const fs::path &output,
                              const BuildDatabase::ObjectFileInfo &compilationUnitInfo);

        fs::path getTestExecutablePath(const fs::path &sourcePath) const;

        BuildResult addLinkTargetRecursively(const fs::path &unitFile,
                                             const std::string &suffixForParentOfStubs,
                                             bool hasParent,
                                             bool transformExeToLib);

    public:
        NativeMakefilePrinter(utbot::ProjectContext projectContext,
                              shared_ptr<BuildDatabase> buildDatabase,
                              fs::path const &rootPath,
                              fs::path primaryCompiler,
                              CollectionUtils::FileSet const *stubSources);

        NativeMakefilePrinter(const NativeMakefilePrinter &baseMakefilePrinter,
                              const fs::path &sourcePath);

        NativeMakefilePrinter(const BaseTestGen &testGen,
                              CollectionUtils::FileSet const *stubSources);

        void init();

        void close();

        void addLinkTargetRecursively(const fs::path &unitFile,
                                      const string &suffixForParentOfStubs,
                                      bool exeToLib = true);

        void addStubs(const CollectionUtils::FileSet &stubsSet);
    };
}


#endif // UNITTESTBOT_NATIVEMAKEFILEPRINTER_H
