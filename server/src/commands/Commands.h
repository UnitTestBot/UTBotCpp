#ifndef UNITTESTBOT_COMMANDS_H
#define UNITTESTBOT_COMMANDS_H

#include "Paths.h"

#include "loguru.h"

#include <protobuf/testgen.grpc.pb.h>

#include <CLI11.hpp>
#include <string>

using namespace ::testsgen;

namespace Commands {
    extern uint32_t threadsPerUser;
    extern uint32_t kleeProcessNumber;

    struct MainCommands {
        explicit MainCommands(CLI::App &app);

        CLI::App *getServerCommand();

        CLI::App *getGenerateCommand();

        CLI::App *getRunTestsCommand();

        CLI::App *getAllCommand();

        fs::path getLogPath();

        loguru::NamedVerbosity getVerbosity();

    private:
        loguru::NamedVerbosity verbosity = loguru::Verbosity_INFO;
        fs::path logPath;
        static const std::map<std::string, loguru::NamedVerbosity> verbosityMap;

        CLI::App *serverCommand;
        CLI::App *generateCommand;
        CLI::App *runTestsCommand;
        CLI::App *allCommand;
    };

    struct ServerCommandOptions {
        explicit ServerCommandOptions(CLI::App *command);

        unsigned int getPort();

        unsigned int getThreadsPerUser();

        unsigned int getKleeProcessNumber();
    private:
        unsigned int port = 0;
    };


    struct GenerateCommands {
        explicit GenerateCommands(CLI::App *command);

        CLI::App *getProjectCommand();
        CLI::App *getStubsCommand();
        CLI::App *getFolderCommand();
        CLI::App *getFileCommand();
        CLI::App *getSnippetCommand();
        CLI::App *getFunctionCommand();
        CLI::App *getClassCommand();
        CLI::App *getLineCommand();
        CLI::App *getAssertionCommand();
        CLI::App *getPredicateCommand();

        bool gotProjectCommand();
        bool gotStubsCommand();
        bool gotFolderCommand();
        bool gotFileCommand();
        bool gotSnippetCommand();
        bool gotFunctionCommand();
        bool gotClassCommand();
        bool gotLineCommand();
        bool gotAssertionCommand();
        bool gotPredicateCommand();

    private:
        CLI::App *generateCommand;

        CLI::App *projectCommand;
        CLI::App *stubsCommand;
        CLI::App *folderCommand;
        CLI::App *fileCommand;
        CLI::App *snippetCommand;
        CLI::App *functionCommand;
        CLI::App *classCommand;
        CLI::App *lineCommand;
        CLI::App *assertionCommand;
        CLI::App *predicateCommand;
    };


    struct GenerateBaseCommandsOptions {
        [[nodiscard]] std::string getSrcPaths() const;

        [[nodiscard]] std::optional<std::string> getTarget() const;

        // source paths
        std::string srcPaths;
        const std::string srcPathsDescription = "Relative paths to directories, containing source files. "
                                                "Separate each path with comma.";
        const std::string srcPathsFlag = "-s,--src-paths";

        // target
        std::optional<std::string> target;
        const std::string targetDescription = "Name or full path of target.";
        const std::string targetFlag = "--target";
    };

    struct GenerateCommandsOptions : public GenerateBaseCommandsOptions {
        explicit GenerateCommandsOptions(GenerateCommands &generateCommands);

        [[nodiscard]] fs::path getFilePath() const;

        [[nodiscard]] fs::path getFolderPath() const;

        [[nodiscard]] unsigned int getLineNumber() const;

        [[nodiscard]] testsgen::ValidationType getValidationType() const;

        [[nodiscard]] std::string getPredicate() const;

        [[nodiscard]] std::string getReturnValue() const;

    private:
        // file path
        fs::path filePath;
        const std::string filePathDescription = "Relative path to testing source file.";
        const std::string filePathFlag = "-f,--file-path";

        // folder path
        fs::path folderPath;
        const std::string folderPathDescription = "Relative path to testing folder.";
        const std::string folderPathFlag = "-f,--folder-path";

        // line number
        unsigned int lineNumber;
        const std::string lineNumberDescription = "Line number of testing entity in source file.";
        const std::string lineNumberFlag = "-l,--line-number";

        // predicate info

        testsgen::ValidationType type{ testsgen::ValidationType::INT32_T };
        static const std::map<std::string, testsgen::ValidationType> validationTypeMap;
        std::string validationType;
        std::string predicate;
        std::string returnValue;
    };

    struct RunTestsCommands {
        explicit RunTestsCommands(MainCommands &commands);

        CLI::App *getRunTestCommand();

        CLI::App *getRunFileCommand();

        CLI::App *getRunProjectCommand();

        bool gotRunTestCommand();

        bool gotRunFileCommand();

        bool gotRunProjectCommand();

    private:
        CLI::App *runCommand;

        CLI::App *runTestCommand;
        CLI::App *runFileCommand;
        CLI::App *runProjectCommand;
    };

    struct RunTestsCommandOptions {
        explicit RunTestsCommandOptions(RunTestsCommands &commands);

        fs::path getFilePath();

        std::string getTestSuite();

        std::string getTestName();

        [[nodiscard]] bool withCoverage() const;

    private:
        fs::path filePath;
        std::string testSuite;
        std::string testName;

        bool noCoverage = false;
    };

    struct AllCommandOptions : public GenerateBaseCommandsOptions {
        explicit AllCommandOptions(CLI::App *command);

        [[nodiscard]] bool withCoverage() const;

    private:
        CLI::App *allCommand;
        bool noCoverage = false;
    };

    struct ProjectContextOptionGroup {
        explicit ProjectContextOptionGroup(CLI::App *command);

        [[nodiscard]] CLI::Option_group *getProjectContextOptions() const;

        [[nodiscard]] std::string getProjectName() const;

        [[nodiscard]] fs::path getProjectPath() const;

        [[nodiscard]] std::string getTestDirectory() const;

        [[nodiscard]] std::string getBuildDirectory() const;

    private:
        CLI::Option_group *projectContextOptions;
        fs::path projectPath;
        std::string testDir = "tests";
        std::string buildDir = "build";
    };

    struct SettingsContextOptionGroup {
        explicit SettingsContextOptionGroup(CLI::App *command);

        [[nodiscard]] CLI::Option_group *getSettingsCommandsContext() const;

        [[nodiscard]] bool doGenerateForStaticFunctions() const;

        [[nodiscard]] bool isVerbose() const;

        [[nodiscard]] bool isDeterministicSearcherUsed() const;

        [[nodiscard]] int32_t getTimeoutPerFunction() const;

        [[nodiscard]] int32_t getTimeoutPerTest() const;

        [[nodiscard]] bool withStubs() const;

        [[nodiscard]] ErrorMode getErrorMode() const;

        [[nodiscard]] bool doDifferentVariablesOfTheSameType() const;
        [[nodiscard]] bool getSkipObjectWithoutSource() const;

    private:
        CLI::Option_group *settingsContextOptions;
        bool generateForStaticFunctions = true;
        bool verbose = false;
        int32_t timeoutPerFunction = 30;
        int32_t timeoutPerTest = 30;
        bool noDeterministicSearcher = false;
        bool noStubs = false;
        ErrorMode errorMode = ErrorMode::FAILING;
        bool differentVariablesOfTheSameType = false;
        bool skipObjectWithoutSource = false;
    };
};


#endif // UNITTESTBOT_COMMANDS_H
