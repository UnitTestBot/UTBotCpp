/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Paths.h"

#include "ProjectContext.h"
#include "utils/StringUtils.h"

#include <pwd.h>
#include <unistd.h>

namespace Paths {
    static fs::path getHomeDir() {
        const char *homeDir;
        if ((homeDir = getenv("HOME")) == nullptr) {
            homeDir = getpwuid(getuid())->pw_dir;
        }
        return fs::canonical(fs::path(homeDir));
    }

    fs::path logPath = getHomeDir();
    fs::path tmpPath = getHomeDir();

    vector<fs::path> filterPathsByDirNames(const vector<fs::path> &paths,
                                           const vector<fs::path> &dirPaths,
                                           const std::function<bool(const fs::path &path)> &filter) {
        std::vector<fs::path> filtered;
        std::copy_if(paths.begin(), paths.end(), std::back_inserter(filtered),
                     [&dirPaths, &filter](const fs::path &path) {
                         return std::any_of(
                             dirPaths.begin(), dirPaths.end(), [&](const auto &dirPath) {
                                 return path.parent_path() == dirPath && fs::exists(path) &&
                                        filter(path);
                             });
                     });
        return filtered;
    }

    CollectionUtils::FileSet pathsToSet(const vector<fs::path> &paths) {
        CollectionUtils::FileSet pathSet;
        for (const auto &p : paths) {
            pathSet.insert(p);
        }
        return pathSet;
    }
    bool isSubPathOf(const fs::path &base, const fs::path &sub) {
        auto s = sub.parent_path();
        auto m = std::mismatch(base.begin(), base.end(), s.begin(), s.end());
        return m.first == base.end();
    }

    fs::path longestCommonPrefixPath(const fs::path &a, const fs::path &b) {
        if (a == b) {
            return a;
        }
        auto const &[mismatchA, mismatchB] = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
        fs::path result =
            std::accumulate(a.begin(), mismatchA, fs::path{},
                            [](fs::path const &a, fs::path const &b) { return a / b; });
        return result;
    }


    CollectionUtils::FileSet findFilesInFolder(const fs::path &folder) {
        if (!fs::exists(folder)) {
            return {};
        }
        CollectionUtils::FileSet moduleFiles;
        for (const auto &entry : fs::recursive_directory_iterator(folder)) {
            if (entry.is_regular_file()) {
                moduleFiles.insert(entry.path());
            }
        }
        return moduleFiles;
    }

    vector<fs::path> findFilesInFolder(const fs::path &folder, const CollectionUtils::FileSet &sourcePaths) {
        vector<fs::path> moduleFiles;
        for (const auto &entry : fs::recursive_directory_iterator(folder)) {
            if (entry.is_regular_file() && CollectionUtils::contains(sourcePaths, entry.path())) {
                moduleFiles.push_back(entry.path());
            }
        }
        return moduleFiles;
    }

    string mangle(const fs::path& path) {
        string result = path.string();
        StringUtils::replaceAll(result, '.', '_');
        StringUtils::replaceAll(result, '/', '_');
        StringUtils::replaceAll(result, '-', '_');
        return result;
    }

    fs::path subtractPath(string path1, string path2) {
        if (path2 == ".") {
            return path1;
        }
        removeBackTrailedSlash(path1);
        removeBackTrailedSlash(path2);
        auto path3Size = path1.find(path2); // TODO: throw if path1 doesn't end with path2
        auto path3 = path1.substr(0, path3Size);
        return path3;
    }

    fs::path getCCJsonFileFullPath(const string &filename, const fs::path &directory) {
        fs::path path1 = fs::path(filename);
        fs::path path2 = fs::weakly_canonical(directory / path1);
        return fs::exists(path2) ? path2 : path1;
    }

    bool isPath(const string &possibleFilePath) noexcept {
        try {
            return fs::exists(possibleFilePath);
        } catch (...) {
            return false;
        }
    }

    //region klee

    static bool errorFileExists(const fs::path &path, std::string const& suffix) {
        fs::path file = replaceExtension(
            path, StringUtils::stringFormat(".%s.err", suffix));
        return fs::exists(file);
    }

    bool hasInternalError(const fs::path &path) {
        static const auto internalErrorSuffixes = {
            "exec",
            "external",
            "xxx"
        };
        return std::any_of(internalErrorSuffixes.begin(), internalErrorSuffixes.end(),
                           [&path](auto const &suffix) { return errorFileExists(path, suffix); });
    }

    bool hasError(const fs::path &path) {
        static const auto internalErrorSuffixes = {
            "abort",
            "assert",
            "bad_vector_access",
            "free",
            "model",
            "overflow",
            "undefined_behavior",
            "ptr",
            "readonly",
            "reporterror",
            "user",
            "uncaught_exception",
            "unexpected_exception"
        };
        return std::any_of(internalErrorSuffixes.begin(), internalErrorSuffixes.end(),
                           [&path](auto const &suffix) { return errorFileExists(path, suffix); });
    }

    //endregion

    //region artifacts
    fs::path createTemporaryObjectFile(const fs::path &output, const fs::path &sourcePath) {
        return replaceExtension(replaceFilename(output, sourcePath.filename()), ".o");
    }

    fs::path getArtifactsRootDir(const utbot::ProjectContext &projectContext) {
        return projectContext.buildDir / "utbot";
    }
    fs::path getFlagsDir(const utbot::ProjectContext &projectContext) {
        return getArtifactsRootDir(projectContext) / "flags";
    }
    fs::path getTestExecDir(const utbot::ProjectContext &projectContext) {
        return getArtifactsRootDir(projectContext) / "tests";
    }
    fs::path getMakefileDir(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath) {
        return getArtifactsRootDir(projectContext) / "make" / getRelativeDirPath(projectContext, sourceFilePath);
    }
    fs::path getGeneratedHeaderDir(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath) {
        return projectContext.testDirPath / getRelativeDirPath(projectContext, sourceFilePath);
    }
    fs::path getRecompiledDir(const utbot::ProjectContext &projectContext) {
        return getTmpDir(projectContext.projectName) / "recompiled";
    }
    fs::path getTestObjectDir(const utbot::ProjectContext &projectContext) {
        return getTmpDir(projectContext.projectName) / "test_objects";
    }
    fs::path getCoverageDir(const utbot::ProjectContext &projectContext) {
        return getTmpDir(projectContext.projectName) / "coverage";
    }
    fs::path getClangCoverageDir(const utbot::ProjectContext &projectContext) {
        return getCoverageDir(projectContext) / "lcov";
    }
    fs::path getGccCoverageDir(const utbot::ProjectContext &projectContext) {
        return getCoverageDir(projectContext) / "gcov";
    }
    fs::path getTestExecutable(const utbot::ProjectContext &projectContext,
                                const fs::path &filePath) {
        return getTestExecDir(projectContext) / filePath.stem();
    }
    fs::path getGeneratedHeaderPath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath) {
        auto headerDir = getGeneratedHeaderDir(projectContext, sourceFilePath);
        return headerDir / replaceExtension(Paths::sourcePathToTestName(sourceFilePath), ".h");
    }
    fs::path getRecompiledFile(const utbot::ProjectContext &projectContext,
                               const fs::path &filePath) {
        fs::path newFilename;
        if (isSourceFile(filePath)) {
            newFilename = fs::relative(filePath, projectContext.projectPath);
            newFilename = addExtension(newFilename, ".o");
        } else {
            newFilename = fs::relative(filePath, projectContext.buildDir);
        }
        return getRecompiledDir(projectContext) / newFilename;
    }
    fs::path getProfrawFilePath(const utbot::ProjectContext &projectContext, const string &testName) {
        return getClangCoverageDir(projectContext) / addExtension(testName, ".profraw");
    }
    fs::path getMainProfdataPath(const utbot::ProjectContext &projectContext) {
        return getClangCoverageDir(projectContext) / "main.profdata";
    }
    fs::path getCoverageJsonPath(const utbot::ProjectContext &projectContext) {
        return getClangCoverageDir(projectContext) / "coverage.json";
    }

    fs::path getGcdaDirPath(const utbot::ProjectContext &projectContext) {
        return getRecompiledDir(projectContext);
    }

    fs::path getBuildFilePath(const utbot::ProjectContext &projectContext,
                              const fs::path &sourceFilePath) {
        fs::path path = getTmpDir(projectContext.projectName) /
                        getRelativeDirPath(projectContext, sourceFilePath) /
                        sourceFilePath.filename();
        return addExtension(path, ".o");
    }

    fs::path getStubBuildFilePath(const utbot::ProjectContext &projectContext,
                                  const fs::path &sourceFilePath) {
        fs::path path = getTmpDir(projectContext.projectName) /
                        getRelativeDirPath(projectContext, sourceFilePath) /
                        sourcePathToStubName(sourceFilePath);
        return addExtension(path, ".o");
    }

    fs::path getWrapperDirPath(const utbot::ProjectContext &projectContext) {
        return getArtifactsRootDir(projectContext) / "wrapper";
    }

    fs::path getWrapperFilePath(const utbot::ProjectContext &projectContext,
                                const fs::path &sourceFilePath) {
        fs::path relative = getRelativeDirPath(projectContext, sourceFilePath);
        fs::path filename = addSuffix(sourceFilePath.filename(), "_wrapper");
        return getWrapperDirPath(projectContext) / relative / filename;
    }


    //endregion

    //region transformation
    static const std::string MAKEFILE_EXTENSION = ".mk";
    static const std::string TEST_SUFFIX = "_test";
    static const std::string STUB_SUFFIX = "_stub";

    fs::path sourcePathToTestPath(const utbot::ProjectContext &projectContext,
                                  const fs::path &sourceFilePath) {
        return projectContext.testDirPath / getRelativeDirPath(projectContext, sourceFilePath) /
               sourcePathToTestName(sourceFilePath);
    }
    fs::path sourcePathToTestName(const fs::path &source) {
        return replaceExtension(addSuffix(source, TEST_SUFFIX), ".cpp").filename();
    }
    fs::path testPathToSourceName(const fs::path &testFilePath) {
        return replaceExtension(removeSuffix(testFilePath, TEST_SUFFIX), ".c").filename();
    }
    fs::path sourcePathToStubName(const fs::path &source) {
        return addSuffix(source, STUB_SUFFIX).filename();
    }
    fs::path getStubBitcodeFilePath(const fs::path &bitcodeFilePath) {
        return Paths::addSuffix(bitcodeFilePath, STUB_SUFFIX);
    }

    fs::path sourcePathToStubHeaderPath(const utbot::ProjectContext &projectContext,
                                  const fs::path &source) {
        return replaceExtension(sourcePathToStubPath(projectContext, source), ".h");
    }

    fs::path sourcePathToHeaderInclude(const fs::path &source) {
        return replaceExtension(sourcePathToStubName(source), ".h");
    }


    fs::path sourcePathToStubPath(const utbot::ProjectContext &projectContext,
                                  const fs::path &source) {
        return normalizedTrimmed((projectContext.testDirPath / "stubs" / getRelativeDirPath(projectContext, source) /
               sourcePathToStubName(source)));
    }
    fs::path testPathToSourcePath(const utbot::ProjectContext &projectContext,
                                  const fs::path &testFilePath) {
        fs::path relative = fs::relative(testFilePath.parent_path(), projectContext.testDirPath);
        fs::path filename = testPathToSourceName(testFilePath);
        return projectContext.projectPath / relative / filename;
    }
    fs::path getMakefilePathFromSourceFilePath(const utbot::ProjectContext &projectContext,
                                               const fs::path &sourceFilePath) {
        fs::path makefileDir = getMakefileDir(projectContext, sourceFilePath);
        string makefileName = replaceExtension(sourceFilePath, MAKEFILE_EXTENSION).filename();
        return makefileDir / makefileName;
    }

    fs::path getStubsMakefilePath(const utbot::ProjectContext &projectContext,
                                  const fs::path &sourceFilePath) {
        fs::path makefileDir = getMakefileDir(projectContext, sourceFilePath);
        string makefileName =
            addExtension(addSuffix(sourceFilePath.stem(), STUB_SUFFIX), MAKEFILE_EXTENSION);
        return makefileDir / makefileName;
    }

    std::optional<fs::path> headerPathToSourcePath(const fs::path &source) {
        if (Paths::isHeaderFile(source)) {
            for (const std::string &extension : CXXFileExtensions) {
                fs::path sourceFilePath = replaceExtension(source, extension);
                if (fs::exists(sourceFilePath)) {
                    return {sourceFilePath};
                }
            }
        }
        return std::nullopt;
    }


    fs::path getRelativeDirPath(const utbot::ProjectContext &projectContext,
                                const fs::path &source) {
        return fs::relative(source.parent_path(), projectContext.projectPath);
    }

    fs::path stubPathToSourcePath(const utbot::ProjectContext &projectContext,
                                  const fs::path &stubPath) {
        fs::path sourceFilePath =
            projectContext.projectPath /
            fs::relative(stubPath, getStubsDirPath(projectContext));
        return removeSuffix(sourceFilePath, STUB_SUFFIX);
    }

    bool isHeadersEqual(const fs::path &srcPath, const fs::path &headerPath) {
        return removeSuffix(srcPath, STUB_SUFFIX).stem() == headerPath.stem();
    }
    fs::path getBuildDir(const utbot::ProjectContext &projectContext) {
        return getTmpDir(projectContext.projectName) / "build";
    }

    //endregion

    const std::vector<std::string> CXXFileExtensions({".cc", ".cp", ".cpp", ".c++", ".cxx"});
    const std::vector<std::string> HPPFileExtensions({".hh", ".hpp", ".hxx"});
    const std::vector<std::string> CFileSourceExtensions({".c"});
    const std::vector<std::string> CFileHeaderExtensions({".h"});

}