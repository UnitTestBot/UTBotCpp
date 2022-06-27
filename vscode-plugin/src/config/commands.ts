export class Commands {

    /**
     * Commands for project configuration
     */
    public static ConfigureProject = 'unittestbot.configureProject';
    public static ReConfigureProject = 'unittestbot.reConfigureProject';

    /**
     * Commands for tests generation
     */
    public static GenerateTestsForIsolatedFile = 'unittestbot.generateIsolatedFileTests';
    public static GenerateTestsForProject = 'unittestbot.generateProjectTests';
    public static GenerateTestsForFolder = 'unittestbot.generateFolderTests';
    public static GenerateTestsForFile = 'unittestbot.generateFileTests';
    public static GenerateTestsForFunction = 'unittestbot.generateFunctionTests';
    public static GenerateTestsForClass = 'unittestbot.generateClassTests';
    public static GenerateProjectLineTests = 'unittestbot.generateProjectLineTests';
    public static GenerateAssertionFailTests = 'unittestbot.generateAssertionFailTests';
    public static GeneratePredicateTests = 'unittestbot.generatePredicateTests';

    /**
     * Commands for tests generation. Short naming versions. Used as commands for menu items
     */
    public static GenerateTestsForFileMenu = 'unittestbot.menucommand.generateFileTests';
    public static GenerateTestsForFunctionMenu = 'unittestbot.menucommand.generateFunctionTests';
    public static GenerateTestsForClassMenu = 'unittestbot.menucommand.generateClassTests';
    public static GenerateProjectLineTestsMenu = 'unittestbot.menucommand.generateProjectLineTests';
    public static GenerateAssertionFailTestsMenu = 'unittestbot.menucommand.generateAssertionFailTests';
    public static GeneratePredicateTestsMenu = 'unittestbot.menucommand.generatePredicateTests';

    public static MenuCommands = [
        Commands.GenerateTestsForFileMenu,
        Commands.GenerateTestsForFunctionMenu,
        Commands.GenerateTestsForClassMenu,
        Commands.GenerateProjectLineTestsMenu,
        Commands.GenerateAssertionFailTestsMenu,
        Commands.GeneratePredicateTestsMenu
    ];

    /**
    * Commands related to test running
    */
    public static RunAllTestsAndShowCoverage = 'unittestbot.runTestsAndShowCoverage';
    public static RunSpecificTestAndShowCoverage = 'unittestbot.runSpecificTestAndShowCoverage';
    public static HideCoverageGutters = 'unittestbot.hideCoverage';
    public static GenerateStubsForProject = 'unittestbot.generateStubsForProject';

    /**
     * Commands related to UTBot settings
     */
    public static SelectLoggingLevel = 'unittestbot.selectLoggingLevel';
    public static UpdateVerboseTestFlag = 'unittestbot.updateVerboseTestFlag';
    // public static ReconnectToServer = 'unittestbot.reconnectToServer';

    /**
     * Commands related to 'UTBot Targets' view
     */
     public static RefreshUTBotTargetsView = 'unittestbot.utbottargets.refreshEntry';
     public static AddUTBotTarget = 'unittestbot.utbottargets.useTargetEntry';
     public static AddUTBotTargetPath = 'unittestbot.utbottargets.useTargetEntryPath';

     public static UTbotTargetsCommands = [
        Commands.RefreshUTBotTargetsView,
        Commands.AddUTBotTarget,
        Commands.AddUTBotTargetPath
    ];

    /**
     * Commands related to 'UTBot Folders' view
     */
    public static RefreshUTBotSourceFoldersView = 'unittestbot.utbotfolders.refreshEntry';
    public static AddUTBotSourceFolder = 'unittestbot.utbotfolders.useFolderEntry';
    public static DeleteUTBotSourceFolder = 'unittestbot.utbotfolders.stopUseFolderEntry';

    public static UTbotFoldersCommands = [
        Commands.RefreshUTBotSourceFoldersView,
        Commands.AddUTBotSourceFolder,
        Commands.DeleteUTBotSourceFolder
    ];
    
    /**
     * Commands related to UTBot Wizard
     */
    public static InitWizardWebview = 'unittestbot.wizard.init';

    /**
     * Developer mode commands
     */
    public static PrintModulesContent = 'unittestbot.printModulesContent';

    /**
     * Other commands
     */
    public static ShowAllCommands = 'unittestbot.showAllCommands';
    public static OpenBuildDirectory = 'unittestbot.innercommand.openBuildDirectoryConfig';
}
