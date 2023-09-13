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
#include <vector>
#include <unordered_set>

namespace Paths {
    extern fs::path logPath;
    const std::string MAKEFILE_EXTENSION = ".mk";
    const std::string CXX_EXTENSION = ".cpp";
    const std::string TEST_SUFFIX = "_test";
    const std::string STUB_SUFFIX = "_stub";
    const std::string DOT_SEP = "_dot_";
    const std::string MAKE_WRAPPER_SUFFIX = "_wrapper";
    const char dot = '.';

    //region util
    static inline bool isValidDir(const std::string &dir) {
        fs::path p(dir);
        return (!fs::exists(p) || fs::is_directory(p));
    }

    bool testInputFile(const std::string &fileName);

    /**
     * @brief Traverses all paths and removes all which parent directory is not any of `dirPaths`.
     * @param path Set of paths to files.
     * @param dirPaths Vector of directory paths where files must be located.
     * @param allowedExt If file extension not present in `allowedExt`, skips it.
     * @return Vector of filtered paths.
     */
    CollectionUtils::FileSet
    filterPathsByDirNames(const CollectionUtils::FileSet &path,
                          const std::vector<fs::path> &dirNames,
                          const std::function<bool(const fs::path &path)> &filter);

    bool errorFileExists(const fs::path &path, std::string const& suffix);

    static inline void setOptPath(fs::path &path, const std::string &value) {
        path = fs::path(value);
    }

    static inline fs::path addExtension(const std::string &filePath, const std::string &extension) {
        return filePath + extension;
    }

    static inline fs::path
    createNewDirForFile(const fs::path &file, const fs::path &oldBase, const fs::path &newBase) {
        fs::path result = newBase / fs::relative(file, oldBase);
        fs::create_directories(result.parent_path());
        return result;
    }

    static inline void removeBackTrailedSlash(std::string &pathStr) {
        if (!pathStr.empty() && pathStr.back() == '/') {
            pathStr.pop_back();
        }
    }

    // returns path3: path1 == path3/path2/...
    fs::path subtractPath(std::string path1, std::string path2);

    static inline fs::path addPrefix(const fs::path &path, std::string const &prefix) {
        return path.parent_path() / (prefix + path.filename().string());
    }

    static inline fs::path addSuffix(const fs::path &path, std::string const &suffix) {
        return path.parent_path() / (path.stem().string() + suffix + path.extension().string());
    }

    static inline fs::path addTestSuffix(const fs::path &path) {
        return addSuffix(path, Paths::TEST_SUFFIX);
    }

    static inline fs::path removeSuffix(const fs::path &path, std::string const &suffix) {
        std::string filenameWithoutExt = path.stem().string();
        std::size_t pos = filenameWithoutExt.rfind(suffix);
        if (pos != std::string::npos) {
            filenameWithoutExt.replace(pos, suffix.length(), "");
        }
        return path.parent_path() / (filenameWithoutExt + path.extension().string());
    }

    CollectionUtils::FileSet pathsToSet(const std::vector<fs::path> &paths);

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

    CollectionUtils::FileSet findFilesInFolder(const fs::path &folder);

    std::vector<fs::path> findFilesInFolder(const fs::path &folder, const CollectionUtils::FileSet &sourcePaths);

    std::string mangle(const fs::path& path);

    std::string mangleExtensions(const fs::path& path);

    static inline fs::path addOrigExtensionAsSuffixAndAddNew(const fs::path &path,
                                                             const std::string &newExt) {
        std::string extensionAsSuffix = path.extension().string();
        if (!extensionAsSuffix.empty()) {
            std::string fnWithNewExt =
                    path.stem().string() + DOT_SEP + extensionAsSuffix.substr(1) + newExt;
            return path.parent_path() / fnWithNewExt;
        }
        return replaceExtension(path, newExt);
    }

    static inline fs::path restoreExtensionFromSuffix(const fs::path &path,
                                                      const std::string &defaultExt) {
        std::string fnWithoutExt = path.stem();
        fs::path fnWithExt;
        std::size_t posEncodedExtension = fnWithoutExt.rfind(DOT_SEP);
        if (posEncodedExtension == std::string::npos) {
            // In `sample_class_test.cpp` the `class` is not an extension
            fnWithExt = fnWithoutExt + defaultExt;
        }
        else {
            // In `sample_class_dot_cpp.cpp` the `cpp` is an extension
            fnWithExt = fnWithoutExt.substr(0, posEncodedExtension)
                        + dot
                        + fnWithoutExt.substr(posEncodedExtension + DOT_SEP.length());
        }
        return path.parent_path() / fs::path(fnWithExt);
    }

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

    static inline fs::path getUtbotLogAllFilePath() {
        const static std::string filename = "utbot-" + TimeUtils::getCurrentTimeStr() + ".log";
        return logPath / Paths::getBaseLogDir() / filename;
    }

    static inline fs::path getClientLogDir(const std::string &client) {
        return getBaseLogDir() / client;
    }

    static inline fs::path getLogDir(const std::string &projectName = "") {
        if (projectName.empty()) {
            return getBaseLogDir() / RequestEnvironment::getClientId();
        }
        return getBaseLogDir() / RequestEnvironment::getClientId() / projectName;
    }

    static inline fs::path getExecLogPath(const std::string &projectName) {
        fs::path execLogPath = getLogDir(projectName);
        auto logFilename = TimeUtils::getDate() + ".log";
        execLogPath /= logFilename;
        return execLogPath;
    }

    static inline fs::path getSymLinkPathToLogLatest() {
        return Paths::getBaseLogDir() / "latest.log";
    }

    //endregion

    static inline fs::path getUTBotFiles(const utbot::ProjectContext &projectContext) {
        return projectContext.buildDir() / CompilationUtils::UTBOT_FILES_DIR_NAME;
    }

    static inline fs::path getUTBotBuildDir(const utbot::ProjectContext &projectContext) {
        return getUTBotFiles(projectContext) / CompilationUtils::UTBOT_BUILD_DIR_NAME;
    }

    static inline fs::path getRelativeUtbotBuildDir(const utbot::ProjectContext &projectContext) {
        return fs::relative(getUTBotBuildDir(projectContext), projectContext.projectPath);
    }

    //region json
    static inline fs::path getClientsJsonPath() {
        return getBaseLogDir() / "clients.json";
    }

    fs::path getCCJsonFileFullPath(const std::string &filename, const fs::path &directory);

    bool isPath(const std::string &possibleFilePath) noexcept;
    //endregion

    //region klee
    static inline fs::path getBitcodeOutFilePath(const fs::path &projectTmpPath, const std::string &filePath) {
        return projectTmpPath / replaceExtension(filePath, ".bc");
    }

    static inline fs::path getLinkedBitcodeFilePath(const fs::path &projectTmpPath) {
        return (projectTmpPath / "out.bc").string();
    }

    static inline fs::path getKleeTmpLogFilePath() {
        return getBaseLogDir() / "klee_tmp_log.txt";
    }

    static inline fs::path getKleeOutDir(const utbot::ProjectContext &projectContext) {
        return getUTBotFiles(projectContext) / "klee_out";
    }

    static inline bool isKtest(fs::path const &path) {
        return path.extension() == ".ktest";
    }

    static inline bool isKtestJson(fs::path const &path) {
        return path.extension() == ".ktestjson";
    }

    static inline bool hasEarly(fs::path const &path) {
        auto earlyPath = replaceExtension(path, ".early");
        return fs::exists(earlyPath);
    }

    bool hasInternalError(fs::path const &path);

    std::vector<fs::path> getErrorDescriptors(fs::path const &path);

    fs::path kleeOutDirForFilePath(const utbot::ProjectContext &projectContext, const fs::path &filePath);

    fs::path kleeOutDirForEntrypoints(const utbot::ProjectContext &projectContext,
                                      const fs::path &srcFilePath,
                                      const std::string &methodNameOrEmptyForFolder);

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

    fs::path getGTestResultsJsonPath(const utbot::ProjectContext &projectContext);

    fs::path getFlagsDir(const utbot::ProjectContext &projectContext);

    fs::path getTestExecDir(const utbot::ProjectContext &projectContext);

    fs::path getMakefileDir(const utbot::ProjectContext &projectContext,
                            const fs::path &sourceFilePath);

    fs::path getGeneratedHeaderDir(const utbot::ProjectContext &projectContext,
                                   const fs::path &sourceFilePath);

    fs::path getPathDirRelativeToTestDir(const utbot::ProjectContext &projectContext,
                                         const fs::path &sourceFilePath);

    fs::path getPathDirRelativeToBuildDir(const utbot::ProjectContext &projectContext,
                                          const fs::path &sourceFilePath);

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

    std::optional<std::string> getRelativePathWithShellVariable(const fs::path &shellVariableForBase,
                                                             const std::string &base,
                                                             const std::string &source);

    fs::path
    getMakefilePathFromSourceFilePath(const utbot::ProjectContext &projectContext, const fs::path &sourceFilePath,
                                      const std::string &suffix = "");

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

    bool hasUncaughtException(const fs::path &path);
    //endregion

    //region utbot_report

    const std::string UTBOT_REPORT = "utbot_report";

    inline fs::path getUTBotReportDir(const utbot::ProjectContext &projectContext) {
        return projectContext.projectPath / UTBOT_REPORT;
    }

    inline fs::path getGenerationStatsCSVPath(const utbot::ProjectContext &projectContext) {
        return getUTBotReportDir(projectContext) / "generation-stats.csv";
    }
    inline fs::path getExecutionStatsCSVPath(const utbot::ProjectContext &projectContext) {
        return getUTBotReportDir(projectContext) / "execution-stats.csv";
    }

    //endregion

    bool isHeadersEqual(const fs::path &srcPath, const fs::path &headerPath);
} // Paths

#endif //UNITTESTBOT_PATHS_H
