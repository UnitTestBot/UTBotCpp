#ifndef UNITTESTBOT_KLEEGENERATOR_H
#define UNITTESTBOT_KLEEGENERATOR_H

#include "PathSubstitution.h"
#include "ProjectContext.h"
#include "Result.h"
#include "SettingsContext.h"
#include "Tests.h"
#include "building/BuildDatabase.h"
#include "exceptions/CompilationDatabaseException.h"
#include "printers/KleePrinter.h"
#include "printers/TestsPrinter.h"
#include "streams/DummyStreamWriter.h"
#include "streams/WriterUtils.h"
#include "streams/tests/TestsWriter.h"
#include "types/Types.h"
#include "utils/ExecUtils.h"
#include "utils/path/FileSystemPath.h"
#include "testgens/BaseTestGen.h"

#include <optional>
#include <sstream>
#include <string>


using json = nlohmann::json;

/**
 * @brief Contains methods for generating files for KLEE and build them.
 */
class KleeGenerator {
    using MethodKtests = tests::MethodKtests;
    using TestsMap = tests::TestsMap;

public:
    /**
     * @brief Also creates tmp directories for build files.
     * @param _testGen contains context for current request.
     * @param typesHandler provides additional information about types.
     * @param filePathsSubstitution Mapping from source file path to modified file. Required for
     * line test generation requests.
     * @param buildDatabase Instance of BuildDatabase which handles link and compile commands
     * @throws fs::filesystem_error Thrown if it can't create tmp folder for some
     * reasons.
     */
    KleeGenerator(BaseTestGen &_testGen, types::TypesHandler &typesHandler,
                  PathSubstitution filePathsSubstitution);

    struct BuildFileInfo {
        fs::path outFilePath;
        fs::path srcFilePath;
    };

    /**
     * @brief Builds source files from compile_commands.json
     *
     * Traverses files listed in compilation database, if file path not in `sourcesFilePaths`, skips
     * it. Else, removes all optimization flags from the compile command, adds own debug flags
     * required for Klee and finally builds them into temporary folder.
     * @return Vector of paths to output files "*.bc".
     * @throws ExecutionProcessException if any of Clang calls returns non-zero code.
     */
    std::vector<BuildFileInfo> buildByCDb(const CollectionUtils::MapFileTo<fs::path> &compileCommand,
                                          const CollectionUtils::FileSet &stubSources = {});

    std::vector<KleeGenerator::BuildFileInfo>
    buildByCDb(const CollectionUtils::FileSet &filesToBuild,
               const CollectionUtils::FileSet &stubSources = {});


    Result<fs::path> defaultBuild(const fs::path &hintPath,
                                  const fs::path &sourceFilePath,
                                  const fs::path &buildDirPath = "",
                                  const std::vector<std::string> &flags = {});

    /**
     * @brief Builds source file with default compilation flags.
     *
     * Compilation flags are listed in `Printer::DEFAULT_BUILD_FLAGS`.
     * @param sourceFilePath Path to the source file.
     * @param buildDirPath if passed, build runs from the folder with compile_commands.json
     * @param flags if passed, add flags to build command
     * @return Path to the output file "*.bc".
     * @throws ExecutionProcessException if a Clang call returns non-zero code.
     */
    Result<fs::path> defaultBuild(const fs::path &sourceFilePath,
                                  const fs::path &buildDirPath = "",
                                  const std::vector<std::string> &flags = {});

    /**
     * @brief Writes temporary Klee files and builds them.
     *
     * For every source file creates temporary .c file with signatures of methods from this file.
     * Also writes code required by Klee into it. Finally, builds temporary files with default build
     * flags. If there is lineInfo, it skips all files not containing requested line
     * @param testsMap Global structure with source methods info.
     * @param lineInfo Information about requested line.
     * @param buildDirPath Server build directory.
     * @return Vector of paths to built binary files.
     * @throws ExecutionProcessException if a Clang call returns non-zero code.
     */
    std::vector<fs::path> buildKleeFiles(const TestsMap &testsMap,
                                         const std::shared_ptr<LineInfo> &lineInfo);

    /**
     * @brief Parse JSON chunks into final tests code.
     *
     * The pipeline of the parsing is much too complex and must be divided into smaller functions.
     * @param testsMap Global structure with source methods info.
     * @param kleeOutput Vector of JSON chunks produced by Klee.
     * @param lineInfo Information about requested line in case of line generation scenarios.
     * @throws ExecutionProcessException if a Clang call returns non-zero code.
     */
    void
    parseKTestsToFinalCode(tests::Tests &tests,
                           const std::unordered_map<std::string, types::Type> &methodNameToReturnTypeMap,
                           const std::vector<MethodKtests> &kleeOutput,
                           const std::shared_ptr<LineInfo> &lineInfo = nullptr,
                           bool verbose = false);

    [[nodiscard]] fs::path getBitcodeFile(const fs::path &sourcePath) const;

    void handleFailedFunctions(tests::TestsMap &testsMap);

    /**
     * @brief Provides command line which is built by hint path, stub sources and flags.
     * Contains all necessary settings for build klee files, source path and bitcode path are set
     * automatically.
     *
     * @param hintPath Can be either source's path or link unit's path or something more.
     * It is used to get command line's pattern from compilation database.
     * @param stubSources Set of stubs sources.
     * If there is inferred source path in set, it is replaced with stub.
     * @param flags Additional flags to be added.
     */
    std::optional<utbot::CompileCommand>
    getCompileCommandForKlee(const fs::path &hintPath,
                             const CollectionUtils::FileSet &stubSources,
                             const std::vector<std::string> &flags,
                             bool forStub) const;

    std::vector<utbot::CompileCommand>
    getCompileCommandsForKlee(const CollectionUtils::MapFileTo<fs::path> &filesToBuild,
                              const CollectionUtils::FileSet &stubSources) const;

private:
    BaseTestGen &testGen;
    types::TypesHandler typesHandler;
    PathSubstitution pathSubstitution;

    CollectionUtils::MapFileTo<std::vector<std::string>> failedFunctions;

    fs::path writeKleeFile(
        printer::KleePrinter &kleePrinter,
        Tests const &tests,
        const std::shared_ptr<LineInfo> &lineInfo,
        const std::function<bool(tests::Tests::MethodDescription const &)> &methodFilter =
            [](tests::Tests::MethodDescription const &) { return true; });
};


#endif // UNITTESTBOT_KLEEGENERATOR_H
