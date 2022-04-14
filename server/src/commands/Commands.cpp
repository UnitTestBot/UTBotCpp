#include "Commands.h"

#include "utils/StringUtils.h"
#include "utils/CLIUtils.h"
#include "config.h"

uint32_t Commands::threadsPerUser = 0;
uint32_t Commands::kleeProcessNumber = 0;

Commands::MainCommands::MainCommands(CLI::App &app) {
    app.set_help_all_flag("--help-all", "Expand all help");
    app.add_flag_function("--version", [](int count){
        std::cout << PROJECT_NAME << " " << PROJECT_VERSION << std::endl;
        if (strlen(RUN_INFO)) {
            std::cout << "Build by " << RUN_INFO << std::endl;
        }
        exit(0);
    }, "Get UTBotCpp version and build detail");
    serverCommand = app.add_subcommand("server", "Launch UTBot server.");
    generateCommand =
        app.add_subcommand("generate", "Generate unit tests and/or stubs.")->require_subcommand();
    runTestsCommand = app.add_subcommand("run", "Launch unit tests and generate coverage info.");
    allCommand = app.add_subcommand(
        "all", "Sequential launch of 'generate stubs' -> 'generate project' -> 'run'.");
    app.require_subcommand(0, 1);
}

CLI::App *Commands::MainCommands::getRunTestsCommand() {
    return runTestsCommand;
}

CLI::App *Commands::MainCommands::getServerCommand() {
    return serverCommand;
}

CLI::App *Commands::MainCommands::getGenerateCommand() {
    return generateCommand;
}

CLI::App *Commands::MainCommands::getAllCommand() {
    return allCommand;
}


Commands::ServerCommandOptions::ServerCommandOptions(CLI::App *command) {
    command->add_option("-p,--port", port, "Port server run on.");
    command->add_option("-j", threadsPerUser, "Maximum number of threads per user.");
    command->add_option("--log", logPath, "Path to folder with logs.");
    command->add_option("-v,--verbosity", verbosity, "Logger verbosity.")
        ->type_name(" ENUM:value in {" +
                    StringUtils::joinWith(CollectionUtils::getKeys(verbosityMap), "|") + "}")
        ->transform(CLI::CheckedTransformer(verbosityMap, CLI::ignore_case));
    command->add_option("--klee-process-number", kleeProcessNumber,
                        "Number of threads for KLEE in interactive mode");
}

fs::path Commands::ServerCommandOptions::getLogPath() {
    return logPath;
}

unsigned int Commands::ServerCommandOptions::getPort() {
    return port;
}

loguru::NamedVerbosity Commands::ServerCommandOptions::getVerbosity() {
    return verbosity;
}

unsigned int Commands::ServerCommandOptions::getThreadsPerUser() {
    return threadsPerUser;
}

unsigned int Commands::ServerCommandOptions::getKleeProcessNumber() {
    return kleeProcessNumber;
}

const std::map<std::string, loguru::NamedVerbosity> Commands::ServerCommandOptions::verbosityMap = {
    { "trace", loguru::NamedVerbosity::Verbosity_MAX },
    { "debug", loguru::NamedVerbosity::Verbosity_1 },
    { "info", loguru::NamedVerbosity::Verbosity_INFO },
    { "warning", loguru::NamedVerbosity::Verbosity_WARNING },
    { "error", loguru::NamedVerbosity::Verbosity_ERROR }
};


Commands::GenerateCommands::GenerateCommands(CLI::App *command) {
    generateCommand = command;
    const std::string generateProject = "project";
    const std::string generateStubs = "stubs";
    const std::string generateFolder = "folder";
    const std::string generateFile = "file";
    const std::string generateSnippet = "snippet";
    const std::string generateFunction = "function";
    const std::string generateClass = "class";
    const std::string generateLine = "line";
    const std::string generateAssertion = "assertion";
    const std::string generatePredicate = "predicate";

    projectCommand = generateCommand->add_subcommand("project", "Generate tests for C project.");
    stubsCommand = generateCommand->add_subcommand("stubs", "Generate stubs for project.");
    folderCommand = generateCommand->add_subcommand("folder", "Generate tests for folder.");
    fileCommand = generateCommand->add_subcommand("file", "Generate tests for file in project.");
    snippetCommand = generateCommand->add_subcommand("snippet", "Generate tests for code snippet.");
    functionCommand = generateCommand->add_subcommand("function", "Generate tests for function.");
    classCommand = generateCommand->add_subcommand("class", "Generate tests for C++ class.");
    lineCommand = generateCommand->add_subcommand("line", "Generate tests for line in file.");
    assertionCommand =
        generateCommand->add_subcommand("assertion", "Generate tests that fails assertion.");
    predicateCommand =
        generateCommand->add_subcommand("predicate", "Generate tests with given result.");
}

CLI::App *Commands::GenerateCommands::getProjectCommand() {
    return projectCommand;
}

CLI::App *Commands::GenerateCommands::getStubsCommand() {
    return stubsCommand;
}

CLI::App *Commands::GenerateCommands::getFolderCommand() {
    return folderCommand;
}

CLI::App *Commands::GenerateCommands::getFileCommand() {
    return fileCommand;
}

CLI::App *Commands::GenerateCommands::getSnippetCommand() {
    return snippetCommand;
}

CLI::App *Commands::GenerateCommands::getFunctionCommand() {
    return functionCommand;
}

CLI::App *Commands::GenerateCommands::getLineCommand() {
    return lineCommand;
}

CLI::App *Commands::GenerateCommands::getClassCommand() {
    return classCommand;
}

CLI::App *Commands::GenerateCommands::getAssertionCommand() {
    return assertionCommand;
}

CLI::App *Commands::GenerateCommands::getPredicateCommand() {
    return predicateCommand;
}

bool Commands::GenerateCommands::gotProjectCommand() {
    return generateCommand->got_subcommand(projectCommand);
}

bool Commands::GenerateCommands::gotStubsCommand() {
    return generateCommand->got_subcommand(stubsCommand);
}

bool Commands::GenerateCommands::gotFolderCommand() {
    return generateCommand->got_subcommand(folderCommand);
}

bool Commands::GenerateCommands::gotFileCommand() {
    return generateCommand->got_subcommand(fileCommand);
}

bool Commands::GenerateCommands::gotSnippetCommand() {
    return generateCommand->got_subcommand(snippetCommand);
}

bool Commands::GenerateCommands::gotFunctionCommand() {
    return generateCommand->got_subcommand(functionCommand);
}

bool Commands::GenerateCommands::gotClassCommand() {
    return generateCommand->got_subcommand(classCommand);
}

bool Commands::GenerateCommands::gotLineCommand() {
    return generateCommand->got_subcommand(lineCommand);
}

bool Commands::GenerateCommands::gotAssertionCommand() {
    return generateCommand->got_subcommand(assertionCommand);
}

bool Commands::GenerateCommands::gotPredicateCommand() {
    return generateCommand->got_subcommand(predicateCommand);
}

Commands::GenerateCommandsOptions::GenerateCommandsOptions(GenerateCommands &generateCommands) {
    // source paths
    generateCommands.getProjectCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    generateCommands.getStubsCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    generateCommands.getFileCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    generateCommands.getFolderCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    generateCommands.getLineCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    generateCommands.getFunctionCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    generateCommands.getClassCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    generateCommands.getAssertionCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    generateCommands.getPredicateCommand()->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    // file path
    generateCommands.getSnippetCommand()
        ->add_option(filePathFlag, filePath, filePathDescription)
        ->required();
    generateCommands.getFileCommand()
        ->add_option(filePathFlag, filePath, filePathDescription)
        ->required();
    generateCommands.getLineCommand()
        ->add_option(filePathFlag, filePath, filePathDescription)
        ->required();
    generateCommands.getFunctionCommand()
        ->add_option(filePathFlag, filePath, filePathDescription)
        ->required();
    generateCommands.getClassCommand()
        ->add_option(filePathFlag, filePath, filePathDescription)
        ->required();
    generateCommands.getAssertionCommand()
        ->add_option(filePathFlag, filePath, filePathDescription)
        ->required();
    generateCommands.getPredicateCommand()
        ->add_option(filePathFlag, filePath, filePathDescription)
        ->required();

    // folder path
    generateCommands.getFolderCommand()
        ->add_option(folderPathFlag, folderPath, folderPathDescription)
        ->required();

    // line number
    generateCommands.getLineCommand()
        ->add_option(lineNumberFlag, lineNumber, lineNumberDescription)
        ->required();
    generateCommands.getFunctionCommand()
        ->add_option(lineNumberFlag, lineNumber, lineNumberDescription)
        ->required();
    generateCommands.getClassCommand()
        ->add_option(lineNumberFlag, lineNumber, lineNumberDescription)
        ->required();
    generateCommands.getAssertionCommand()
        ->add_option(lineNumberFlag, lineNumber, lineNumberDescription)
        ->required();
    generateCommands.getPredicateCommand()
        ->add_option(lineNumberFlag, lineNumber, lineNumberDescription)
        ->required();

    // predicate info
    generateCommands.getPredicateCommand()
        ->add_option("--validation-type", type, "Type of predicate values.")
        ->required()
        ->type_name(" ENUM:value in {" +
                    StringUtils::joinWith(CollectionUtils::getKeys(validationTypeMap), "|") + "}")
        ->transform(CLI::CheckedTransformer(validationTypeMap, CLI::ignore_case));
    generateCommands.getPredicateCommand()
        ->add_option("--predicate", predicate, "Predicate string representation.")
        ->required();
    generateCommands.getPredicateCommand()
        ->add_option("--return-value", returnValue, "Expected return value.")
        ->required();

    // target
    generateCommands.getProjectCommand()->add_option(targetFlag, target, targetDescription);
    generateCommands.getFileCommand()->add_option(targetFlag, target, targetDescription);
    generateCommands.getFolderCommand()->add_option(targetFlag, target, targetDescription);
    generateCommands.getLineCommand()->add_option(targetFlag, target, targetDescription);
    generateCommands.getFunctionCommand()->add_option(targetFlag, target, targetDescription);
    generateCommands.getClassCommand()->add_option(targetFlag, target, targetDescription);
    generateCommands.getAssertionCommand()->add_option(targetFlag, target, targetDescription);
    generateCommands.getPredicateCommand()->add_option(targetFlag, target, targetDescription);
}

std::string Commands::GenerateBaseCommandsOptions::getSrcPaths() const {
    return srcPaths;
}

fs::path Commands::GenerateCommandsOptions::getFilePath() const {
    return filePath;
}

fs::path Commands::GenerateCommandsOptions::getFolderPath() const {
    return folderPath;
}

unsigned int Commands::GenerateCommandsOptions::getLineNumber() const {
    return lineNumber;
}

testsgen::ValidationType Commands::GenerateCommandsOptions::getValidationType() const {
    return type;
}

std::string Commands::GenerateCommandsOptions::getPredicate() const {
    return predicate;
}

std::string Commands::GenerateCommandsOptions::getReturnValue() const {
    return returnValue;
}

std::optional<std::string> Commands::GenerateBaseCommandsOptions::getTarget() const {
    return target;
}

const std::map<std::string, testsgen::ValidationType>
    Commands::GenerateCommandsOptions::validationTypeMap = {
        { "int8", testsgen::ValidationType::INT8_T },
        { "int16", testsgen::ValidationType::INT16_T },
        { "int32", testsgen::ValidationType::INT32_T },
        { "int64", testsgen::ValidationType::INT64_T },
        { "uint8", testsgen::ValidationType::UINT8_T },
        { "uint16", testsgen::ValidationType::UINT16_T },
        { "uint32", testsgen::ValidationType::UINT32_T },
        { "uint64", testsgen::ValidationType::UINT64_T },
        { "bool", testsgen::ValidationType::BOOL },
        { "char", testsgen::ValidationType::CHAR },
        { "float", testsgen::ValidationType::FLOAT },
        { "string", testsgen::ValidationType::STRING }
    };

Commands::ProjectContextOptionGroup::ProjectContextOptionGroup(CLI::App *command) {
    projectContextOptions = command->add_option_group("Project context");
    projectContextOptions
        ->add_option("-p,--project-path", projectPath, "Path to testing project root.")
        ->required();

    projectContextOptions->add_option(
        "-t,--tests-dir", testDir, "Relative path to directory in which tests will be generated.",
        true);

    projectContextOptions->add_option(
        "-b,--build-dir", buildDir,
        "Relative path to build directory with compile_commands.json and/or coverage.json.", true);

    projectContextOptions->add_option(
        "--results-dir", resultsDir,
        "Relative path to results directory. Coverage and run results are saved here.", true);
}

CLI::Option_group *Commands::ProjectContextOptionGroup::getProjectContextOptions() const {
    return projectContextOptions;
}

std::string Commands::ProjectContextOptionGroup::getProjectName() const {
    return projectPath.filename();
}

fs::path Commands::ProjectContextOptionGroup::getProjectPath() const {
    if (!projectPath.empty()) {
        return Paths::normalizedTrimmed(fs::absolute(fs::path(projectPath)));
    }
    return projectPath;
}

std::string Commands::ProjectContextOptionGroup::getTestDirectory() const {
    return testDir;
}

std::string Commands::ProjectContextOptionGroup::getBuildDirectory() const {
    return buildDir;
}

std::string Commands::ProjectContextOptionGroup::getResultsDirectory() const {
    return resultsDir;
}

Commands::SettingsContextOptionGroup::SettingsContextOptionGroup(CLI::App *command) {
    settingsContextOptions = command->add_option_group("Settings context");
    settingsContextOptions->add_flag(
        "-g,--generate-for-static", generateForStaticFunctions,
        "True, if you want UTBot to generate tests for static functions.");
    settingsContextOptions->add_flag("-v,--verbose", verbose,
                                     "Set if is required.");
    settingsContextOptions->add_option("--function-timeout", timeoutPerFunction,
                                       "Maximum time (in seconds) is allowed for generation tests "
                                       "per function. Set to non-positive number to disable it.",
                                       true);
    settingsContextOptions->add_option("--test-timeout", timeoutPerTest,
                                       "Maximum time (in seconds) is allowed for running a single "
                                       "test. Set to non-positive number to disable it.",
                                       true);
    settingsContextOptions->add_flag("--no-deterministic-searcher", noDeterministicSearcher,
                                     "Use deterministic searcher to traverse bitcode in the same "
                                     "way every time. It may significantly slow down generation.");
    settingsContextOptions->add_flag("--no-stubs", noStubs,
                                     "True, if you don't want UTBot to use generated stubs from "
                                     "<testsDir>/stubs folder instead real files.");
}

CLI::Option_group *Commands::SettingsContextOptionGroup::getSettingsCommandsContext() const {
    return settingsContextOptions;
}

bool Commands::SettingsContextOptionGroup::doGenerateForStaticFunctions() const {
    return generateForStaticFunctions;
}

bool Commands::SettingsContextOptionGroup::isVerbose() const {
    return verbose;
}

bool Commands::SettingsContextOptionGroup::isDeterministicSearcherUsed() const {
    return !noDeterministicSearcher;
}

int32_t Commands::SettingsContextOptionGroup::getTimeoutPerFunction() const {
    return timeoutPerFunction;
}

int32_t Commands::SettingsContextOptionGroup::getTimeoutPerTest() const {
    return timeoutPerTest;
}

bool Commands::SettingsContextOptionGroup::withStubs() const {
    return !noStubs;
}

Commands::RunTestsCommands::RunTestsCommands(Commands::MainCommands &commands) {
    runCommand = commands.getRunTestsCommand();

    runTestCommand = runCommand->add_subcommand("test", "Run specified test");
    runFileCommand = runCommand->add_subcommand("file", "Run all tests in specified file");
    runProjectCommand = runCommand->add_subcommand("project", "Run all tests for this project");
}

CLI::App *Commands::RunTestsCommands::getRunProjectCommand() {
    return runProjectCommand;
}

CLI::App *Commands::RunTestsCommands::getRunFileCommand() {
    return runFileCommand;
}

CLI::App *Commands::RunTestsCommands::getRunTestCommand() {
    return runTestCommand;
}

bool Commands::RunTestsCommands::gotRunTestCommand() {
    return runCommand->got_subcommand(runTestCommand);
}

bool Commands::RunTestsCommands::gotRunFileCommand() {
    return runCommand->got_subcommand(runFileCommand);
}

bool Commands::RunTestsCommands::gotRunProjectCommand() {
    return runCommand->got_subcommand(runProjectCommand);
}

Commands::RunTestsCommandOptions::RunTestsCommandOptions(Commands::RunTestsCommands &commands) {
    commands.getRunTestCommand()->add_option("--test-suite", testSuite, "Test suite")->required();
    commands.getRunTestCommand()->add_option("--test-name", testName, "Test name")->required();
    commands.getRunTestCommand()
        ->add_option("--file-path", filePath, "Path to test file")
        ->required();
    commands.getRunFileCommand()
        ->add_option("--file-path", filePath, "Path to test file")
        ->required();
    commands.getRunTestCommand()->add_flag("--no-coverage", noCoverage,
                                           "Flag that controls coverage generation.");
    commands.getRunFileCommand()->add_flag("--no-coverage", noCoverage,
                                           "Flag that controls coverage generation.");
    commands.getRunProjectCommand()->add_flag("--no-coverage", noCoverage,
                                              "Flag that controls coverage generation.");
}

fs::path Commands::RunTestsCommandOptions::getFilePath() {
    return filePath;
}

std::string Commands::RunTestsCommandOptions::getTestSuite() {
    return testSuite;
}

std::string Commands::RunTestsCommandOptions::getTestName() {
    return testName;
}

bool Commands::RunTestsCommandOptions::withCoverage() const {
    return !noCoverage;
}

Commands::AllCommandOptions::AllCommandOptions(CLI::App *command) : allCommand(command) {
    allCommand->add_option("--no-coverage", noCoverage, "Flag that controls coverage generation.");
    allCommand->add_option(srcPathsFlag, srcPaths, srcPathsDescription);
    allCommand->add_option(targetFlag, target, targetDescription);
}

bool Commands::AllCommandOptions::withCoverage() const {
    return !noCoverage;
}
