/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_PATHS_H
#define UNITTESTBOT_PATHS_H

#include "Language.h"
#include "ProjectContext.h"
#include "RequestEnvironment.h"
#include "utils/CollectionUtils.h"
#include "utils/CompilationUtils.h"
#include "utils/TimeUtils.h"

#include "utils/path/FileSystemPath.h"
#include <optional>
#include <unordered_set>

namespace Paths {
    using std::string;
    using std::vector;

    extern fs::path logPath, tmpPath;

    //region util
    static inline bool isValidDir(const string &dir) {
        fs::path p(dir);
        return (!fs::exists(p) || fs::is_directory(p));
    }

    /**
     * @brief Traverses all paths and removes all which parent directory is not any of `dirPaths`.
     * @param path Vector of paths to files.
     * @param dirPaths Vector of directory paths where files must be located.
     * @param allowedExt If file extension not present in `allowedExt`, skips it.
     * @return Vector of filtered paths.
     */
    vector<fs::path> filterPathsByDirNames(const vector<fs::path> &path,
                                           const vector<fs::path> &dirNames,
                                           const std::function<bool(const fs::path &path)> &filter);

    static inline void setOptPath(fs::path &path, const string &value) {
        path = fs::path(value);
    }

    static inline fs::path addExtension(const string &filePath, const string &extension) {
        return filePath + extension;
    }

    static inline fs::path
    createNewDirForFile(const fs::path &file, const fs::path &oldBase, const fs::path &newBase) {
        fs::path result = newBase / fs::relative(file, oldBase);
        fs::create_directories(result.parent_path());
        return result;
    }

    static inline void removeBackTrailedSlash(string &pathStr) {
        if (!pathStr.empty() && pathStr.back() == '/') {
            pathStr.pop_back();
        }
    }

    // returns path3: path1 == path3/path2/...
    fs::path subtractPath(string path1, string path2);

    static inline fs::path addPrefix(const fs::path &path, string const &prefix) {
        return path.parent_path() / (prefix + path.filename().string());
    }

    static inline fs::path addSuffix(const fs::path &path, string const &suffix) {
        return path.parent_path() / (path.stem().string() + suffix + path.extension().string());
    }

    static inline fs::path addTestSuffix(const fs::path &path) {
        return addSuffix(path, "_test");
    }

    static inline fs::path removeSuffix(const fs::path &path, string const &suffix) {
        string filenameWithoutExt = path.stem().string();
        std::size_t pos = filenameWithoutExt.rfind(suffix);
        if (pos != string::npos) {
            filenameWithoutExt.replace(pos, suffix.length(), "");
        }
        return path.parent_path() / (filenameWithoutExt + path.extension().string());
    }

    CollectionUtils::FileSet pathsToSet(const vector<fs::path> &paths);

    static inline fs::path normalizedTrimmed(const fs::path &path) {
        auto r = path.lexically_normal();
        if (r.has_filename()) {
            return r;
        }
        return r.parent_path();
    }

    bool isSubPathOf(const fs::path &base, const fs::path &sub);

    fs::path longestCommonPrefixPath(const fs::path &a, const fs::path &b);

    static inline fs::path replaceExtension(const fs::path &path, const std::string &newExt) {
        return fs::path(path).replace_extension(newExt);
    }

    static inline fs::path removeExtension(const fs::path &path) {
        return replaceExtension(path, "");
    }

    static inline fs::path replaceFilename(const fs::path &path, const std::string &newName) {
        return fs::path(path).replace_filename(newName);
    }

    CollectionUtils::FileSet findFilesInFolder(const fs::path& folder);

    vector<fs::path> findFilesInFolder(const fs::path &folder, const CollectionUtils::FileSet &sourcePaths);

    string mangle(const fs::path& path);
    //endregion

    //region includes
    static inline fs::path mathIncludePath() {
        return fs::path("math.h");
    }

    //endregion

    //region logs
    static inline fs::path getBaseLogDir() {
        return logPath / "logs";
    }

    static inline fs::path getClientLogDir(const string &client) {
        return getBaseLogDir() / client;
    }

    static inline fs::path getClientTmpDir(const string &client) {
        return tmpPath / "tmp" / client;
    }

    static inline fs::path getLogDir(const string &projectName = "") {
        if (projectName.empty()) {
            return getBaseLogDir() / RequestEnvironment::getClientId();
        }
        return getBaseLogDir() / RequestEnvironment::getClientId() / projectName;
    }

    static inline fs::path getExecLogPath(const string &projectName) {
        fs::path execLogPath = getLogDir(projectName);
        auto logFilename = TimeUtils::getDate() + ".log";
        execLogPath /= logFilename;
        return execLogPath;
    }

    //endregion

    static inline fs::path getTmpDir(const string &projectName) {
        if (projectName.empty()) {
            return tmpPath / "tmp" / RequestEnvironment::getClientId();
        }
        return tmpPath / "tmp" / RequestEnvironment::getClientId() / projectName;
    }

    fs::path getBuildDir(const utbot::ProjectContext &projectContext);

    //region json
    static inline fs::path getClientsJsonPath() {
        return tmpPath / "tmp" / "clients.json";
    }

    fs::path getCCJsonFileFullPath(const string &filename, const fs::path &directory);

    bool isPath(const std::string& possibleFilePath) noexcept;
    //endregion

    //region klee
    static inline fs::path getBitcodeOutFilePath(const fs::path &projectTmpPath, const string &filePath) {
        return projectTmpPath / replaceExtension(filePath, ".bc");
    }

    static inline fs::path getLinkedBitcodeFilePath(const fs::path &projectTmpPath) {
        return (projectTmpPath / "out.bc").string();
    }

    static inline fs::path getKleeTmpLogFilePath() {
        return getTmpDir("") / "klee_tmp_log.txt";
    }

    static inline fs::path getKleeOutDir(const fs::path &projectTmpPath) {
        return projectTmpPath / "klee_out";
    }

    static inline bool isKtest(fs::path const& path) {
        return path.extension() == ".ktest";
    }

    static inline bool isKtestJson(fs::path const& path) {
        return path.extension() == ".ktestjson";
    }

    static inline bool hasEarly(fs::path const &path) {
        auto earlyPath = replaceExtension(path, ".early");
        return fs::exists(earlyPath);
    }

    bool hasInternalError(fs::path const &path);

    bool hasError(fs::path const &path);

    //endregion

    //region extensions
    extern const std::vector<std::string> CFileHeaderExtensions;
    static inline bool isHFile(const fs::path &path) {
        return CollectionUtils::contains(CFileHeaderExtensions, path.extension());
    }

    extern const std::vector<std::string> CFileSourceExtensions;
    static inline bool isCFile(const fs::path &path) {
        return CollectionUtils::contains(CFileSourceExtensions, path.extension());
    }

    extern const std::vector<std::string> CXXFileExtensions;
    static inline bool isCXXFile(const fs::path &path) {
        return CollectionUtils::contains(CXXFileExtensions, path.extension());
    }

    extern const std::vector<std::string> HPPFileExtensions;
    static inline bool isHppFile(const fs::path &path) {
        return CollectionUtils::contains(HPPFileExtensions, path.extension());
    }

    static inline bool isHeaderFile(const fs::path &path) {
        return isHFile(path) || isHppFile(path);
    }

    static inline bool isSourceFile(const fs::path &path) {
        return isCFile(path) || isCXXFile(path);
    }

    utbot::Language getSourceLanguage(const fs::path &path);

    static inline bool isObjectFile(const fs::path &path) {
        return path.extension() == ".o";
    }

    static inline bool isStaticLibraryFile(const fs::path &path) {
        return path.extension() == ".a";
    }

    static inline bool isSharedLibraryFile(const fs::path &path) {
        auto candidate = CompilationUtils::removeSharedLibraryVersion(path);
        return candidate.extension() == ".so";
    }

    static inline bool isLibraryFile(const fs::path &path) {
        return isStaticLibraryFile(path) || isSharedLibraryFile(path);
    }

    static inline bool isGtest(const fs::path &path) {
        return path.string().find("gtest") != std::string::npos;
    }

    static inline bool isUTBotWrapper(const std::string &filename) {
        return filename.find("utbot/wrapper") != std::string::npos;
    }
    //endregion

    //region artifacts
    fs::path createTemporaryObjectFile(const fs::path &output, const fs::path &sourcePath);

    fs::path getArtifactsRootDir(const utbot::ProjectContext &projectContext);

    fs::path getFlagsDir(const utbot::ProjectContext &projectContext);

    fs::path getTestExecDir(const utbot::ProjectContext &projectContext);

    fs::path getMakefileDir(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);

    fs::path getGeneratedHeaderDir(const utbot::ProjectContext &projectContext, const fs::path& sourceFilePath);

    fs::path getRecompiledDir(const utbot::ProjectContext &projectContext);

    fs::path getTestObjectDir(const utbot::ProjectContext &projectContext);

    fs::path getCoverageDir(const utbot::ProjectContext &projectContext);

    fs::path getClangCoverageDir(const utbot::ProjectContext &projectContext);

    fs::path getGccCoverageDir(const utbot::ProjectContext &projectContext);

    fs::path getTestExecutable(const utbot::ProjectContext &projectContext, const fs::path &filePath);

    fs::path getGeneratedHeaderPath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);

    fs::path getRecompiledFile(const utbot::ProjectContext &projectContext, const fs::path &filePath);

    fs::path getProfrawFilePath(const utbot::ProjectContext &projectContext, const std::string &testName);

    fs::path getMainProfdataPath(const utbot::ProjectContext &projectContext);

    fs::path getCoverageJsonPath(const utbot::ProjectContext &projectContext);

    fs::path getGcdaDirPath(const utbot::ProjectContext &projectContext);

    fs::path getGcdaFilePath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);

    fs::path getBuildFilePath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);

    fs::path getStubBuildFilePath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);

    fs::path getStubBitcodeFilePath(const fs::path &bitcodeFilePath);

    fs::path getWrapperDirPath(const utbot::ProjectContext &projectContext);

    fs::path getWrapperFilePath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);
    //endregion

    //region transformations
    fs::path sourcePathToTestPath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);

    fs::path testPathToSourceName(const fs::path &testFilePath);

    fs::path sourcePathToTestName(const fs::path &source);

    fs::path sourcePathToStubHeaderPath(const utbot::ProjectContext &projectContext,
                                        const fs::path &source);

    fs::path sourcePathToHeaderInclude(const fs::path &source);

    fs::path sourcePathToStubPath(const utbot::ProjectContext &projectContext, const fs::path &source);

    fs::path sourcePathToStubName(const fs::path &source);

    fs::path stubPathToSourcePath(const utbot::ProjectContext &projectContext, const fs::path &stubPath);

    fs::path testPathToSourcePath(const utbot::ProjectContext &projectContext, const fs::path &testFilePath);

    fs::path getRelativeDirPath(const utbot::ProjectContext &projectContext, const fs::path &source);

    fs::path getMakefilePathFromSourceFilePath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);

    fs::path getStubsMakefilePath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath);

    std::optional<fs::path> headerPathToSourcePath(const fs::path &source);
    //endregion

    //region stubs
    static inline fs::path getStubsDirPath(const utbot::ProjectContext &projectContext) {
        return projectContext.testDirPath / "stubs";
    }

    static inline fs::path getStubsRelativeDirPath(const fs::path &relativeTestDirPath) {
        return "stubs" / relativeTestDirPath;
    }

    //endregion

    bool isHeadersEqual(const fs::path& srcPath, const fs::path& headerPath);
}

#endif //UNITTESTBOT_PATHS_H
