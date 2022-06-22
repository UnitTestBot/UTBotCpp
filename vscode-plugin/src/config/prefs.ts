import * as path from 'path';
import * as vs from 'vscode';
import { UTBotFoldersStorage } from "../explorer/utbotFoldersStorage";
import { UTBotProjectTarget } from '../explorer/UTBotProjectTarget';
import { ExtensionLogger } from '../logger';
import * as pathUtils from '../utils/pathUtils';
import * as vsUtils from '../utils/vscodeUtils';
import * as defcfg from './defaultValues';
import * as Randomstring from 'randomstring';
import { SettingsContext } from '../proto-ts/testgen_pb';
import { isWin32 } from '../utils/utils';
const { logger } = ExtensionLogger;

export class Prefs {
    public static EXTENSION_INITIALIZED_FOR_WORKSPACE_KEY = "INITIALIZED_WORKSPACE";
    public static WORKSPACE_UNIQUE_ID = "WORKSPACE_UNIQUE_ID";
    public static WORKSPACE_ID_LENGTH = 4;

    public static PROJECT_TARGETS_KEY = "PROJECT_TARGETS_PATH_KEY";
    public static PRIMARY_TARGET_PATH_KEY = "PRIMARY_TARGET_PATH";

    public static HOST_PREF = 'unittestbot.deployment.utbotHost';
    public static PORT_PREF = 'unittestbot.deployment.utbotPort';
    public static REMOTE_PATH_PREF = 'unittestbot.deployment.remotePath';

    public static BUILD_DIR_PREF = 'unittestbot.paths.buildDirectory';
    public static CMAKE_OPTIONS_PREF = 'unittestbot.paths.cmakeOptions';
    public static TESTS_DIR_PREF = 'unittestbot.paths.testsDirectory';
    public static SOURCE_DIRS_PREF = 'unittestbot.paths.sourceDirectories';

    public static VERBOSE_MODE_PREF = "unittestbot.testsGeneration.verboseFormatting";

    public static USE_STUBS_PREF = 'unittestbot.stubs.useStubs';

    public static GEN_SARIF_PREF = 'unittestbot.analysis.genSarif';

    public static FUNC_TIMEOUT_PREF = 'unittestbot.advanced.timeoutPerFunction';

    public static TEST_TIMEOUT_PREF = 'unittestbot.advanced.timeoutPerTest';

    public static DETERMINISTIC_SEARCHER_PREF = 'unittestbot.advanced.useDeterministicSearcher';

    public static STATIC_FUNCTIONS_PREF = 'unittestbot.testsGeneration.generateForStaticFunctions';

    public static SHOW_TEST_RESULTS_PREF = 'unittestbot.visual.showTestResults';


    public static isRemoteScenario(): boolean {
        const host = Prefs.getAsset(Prefs.HOST_PREF);
        return !(host === '127.0.0.1' || host === 'localhost') || isWin32();
    }

    /**
     * Obtains an unique workspace identifier for UTBot logging.
     * @param context extension's context
     */
     public static obtainClientId(context: vs.ExtensionContext): string {
         let id: string | undefined = context.workspaceState.get(Prefs.WORKSPACE_UNIQUE_ID);
         if (!id) {
             const generateOptions: Randomstring.GenerateOptions =
                 { length: Prefs.WORKSPACE_ID_LENGTH, readable: true, charset: 'alphanumeric', capitalization: 'uppercase' };
             id = Randomstring.generate(generateOptions);
             void context.workspaceState.update(Prefs.WORKSPACE_UNIQUE_ID, id);
         }
         return id as string;
    }

    /**
     * Creates a settingsContext instance from current preferences.
    */
    public static getSettingsContext(): SettingsContext {
        const settingsContext = new SettingsContext();
        settingsContext.setVerbose(Prefs.isVerboseTestModeSet())
        .setGenerateforstaticfunctions(Prefs.generateForStaticFunctions())
        .setTimeoutperfunction(Prefs.timeoutPerFunction())
        .setTimeoutpertest(Prefs.timeoutPerTest())
        .setUsedeterministicsearcher(Prefs.useDeterministicSearcher())
        .setUsestubs(Prefs.useStubs())
        .setGensarif(Prefs.genSarif());
        return settingsContext;
    }


    /**
     * Marks current workspace as initialized for UTBot.
     * @param context extension's context
     */
     public static async markAsInitialized(context: vs.ExtensionContext): Promise<void> {
        void context.workspaceState.update(Prefs.EXTENSION_INITIALIZED_FOR_WORKSPACE_KEY, true);
    }

    /**
     * Marks current workspace as uninitialized for UTBot.
     * @param context extension's context
     */
    public static async markAsUnInitialized(context: vs.ExtensionContext): Promise<void> {
        void context.workspaceState.update(Prefs.EXTENSION_INITIALIZED_FOR_WORKSPACE_KEY, false);
    }

    /**
     * Checks whether the workspace was previously initialized by UTBot.
     * The check is persistent, so the results are valid between the sessions.
     * @param context extension's context
     * @returns true if workspace was earlier initialized by UTBot
     */
    public static isInitialized(context: vs.ExtensionContext): boolean {
        return Boolean(context.workspaceState.get(Prefs.EXTENSION_INITIALIZED_FOR_WORKSPACE_KEY)).valueOf();
    }

    /**
     * Opposite of `isInitialized()`.
     */
    public static isNotInitialized(context: vs.ExtensionContext): boolean {
        return !this.isInitialized(context);
    }

    public static async setPredictedSettings(): Promise<void> {
        logger.debug("Setting default settings");
        await this.setPredictedHost();
        await this.setPredictedPort();
    }

    public static async setPredictedHost(): Promise<void> {
        const predictedHost = defcfg.DefaultConfigValues.getDefaultHost();
        await Prefs.setHost(predictedHost);
        logger.debug(`Host is automatically set to '${predictedHost}'`);
    }

    public static async setPredictedPort(): Promise<void> {
        const predictedPort = defcfg.DefaultConfigValues.getDefaultPort();
        await Prefs.setPort(predictedPort);
        logger.debug(`Port is automatically set to '${predictedPort}'`);
    }

    /**
     * Marks current workspace as unconfigured for UTBot.
     * @param context extension's context
     */
    public static async setTargets(context: vs.ExtensionContext, projectTargets: Array<UTBotProjectTarget>): Promise<void> {
        await context.workspaceState.update(Prefs.PROJECT_TARGETS_KEY, projectTargets);
    }

    /**
     * Checks whether the workspace was previously configured by UTBot.
     * The check is persistent, so the results are valid between the sessions.
     * @param context extension's context
     * @returns true if workspace was earlier configured by UTBot
     */
    public static getTargets(context: vs.ExtensionContext): Array<UTBotProjectTarget> | undefined {
        return context.workspaceState.get<Array<UTBotProjectTarget>>(Prefs.PROJECT_TARGETS_KEY);
    }

    /**
     * Marks current workspace as unconfigured for UTBot.
     * @param context extension's context
     */
     public static async setPrimaryTargetPath(context: vs.ExtensionContext, primaryTargetPath: string): Promise<void> {
        return context.workspaceState.update(Prefs.PRIMARY_TARGET_PATH_KEY, primaryTargetPath);
    }

    /**
     * Checks whether the workspace was previously configured by UTBot.
     * The check is persistent, so the results are valid between the sessions.
     * @param context extension's context
     * @returns true if workspace was earlier configured by UTBot
     */
    public static getPrimaryTargetPath(context: vs.ExtensionContext): string | undefined {
        return context.workspaceState.get<string>(Prefs.PRIMARY_TARGET_PATH_KEY);
    }

    public static getRemoteRoot(): string {
        const root = this.getAsset(Prefs.REMOTE_PATH_PREF);
        return pathUtils.normalizeRawPosixPath(root);
    }

    public static getProjectDirName(): string {
        return path.parse(vsUtils.getProjectDirByOpenedFile().fsPath).name;
    }

    public static getProjectName(): string {
        return this.getProjectDirName();
    }

    public static getLocalBuildDirPath(): [string, string] {
        const buildDir = this.getAsset(Prefs.BUILD_DIR_PREF);
        const root = vsUtils.getProjectDirByOpenedFile();
        return [root.fsPath, buildDir];
    }

    public static getRemoteBuildDirPath(): [string, string] {
        const buildDir = this.getAsset(Prefs.BUILD_DIR_PREF);
        const root = this.getRemoteRoot();
        return [root, buildDir];
    }

    public static getBuildDirPath(): [string, string] {
        if (this.isRemoteScenario()) {
            return this.getRemoteBuildDirPath();
        }
        return this.getLocalBuildDirPath();
    }

    public static getCmakeOptions(): Array<string> {
        return this.getAssetBase(Prefs.CMAKE_OPTIONS_PREF, defcfg.DefaultConfigValues.DEFAULT_CMAKE_OPTIONS);
    }


    public static getTestsDirPath(): string {
        if (this.isRemoteScenario()) {
            return this.getRemoteTestsDirPath();
        }
        return this.getLocalTestsDirPath();
    }

    public static getLocalTestsDirPath(): string {
        const testsDirRelative = this.getTestDirRelativePath();
        const root = vsUtils.getProjectDirByOpenedFile();
        const testsDirPath = pathUtils.fsJoin(root.fsPath, testsDirRelative);
        return testsDirPath;
    }

    public static getRemoteTestsDirPath(): string {
        const testsDirRelative = this.getTestDirRelativePath();
        const root = this.getRemoteRoot();
        const testsDirPath = path.posix.join(root, testsDirRelative);
        return testsDirPath;
    }

    public static getTestDirRelativePath(): string {
        const testDirRaw = this.getAsset(Prefs.TESTS_DIR_PREF);
        return pathUtils.normalizeRawPosixPath(testDirRaw);
    }

    public static getRemoteSourcePaths(): string[] {
        return Array.from(UTBotFoldersStorage.instance
            .getFolders())
            .map(uri => pathUtils.substituteRemotePath(uri));
    }

    public static getLocalSourcePaths(): string[] {
        return Array.from(UTBotFoldersStorage.instance
            .getFolders());
    }

    public static getSourcePathsRootsFromConfig(): string[] {
        const sourceDirRelativePathsRoots =
            this.getAsset(Prefs.SOURCE_DIRS_PREF)
                .split(",")
                .filter(relPath => relPath.length > 0);
        const root = vsUtils.getProjectDirByOpenedFile();

        const sourceDirs = sourceDirRelativePathsRoots.map(sPath => {
            const localPath = sPath.split(path.posix.sep).join(path.sep);
            const fullPath = pathUtils.fsJoin(root.fsPath, localPath);
            return fullPath;
        });

        return sourceDirs;
    }

    public static getSourcePaths(): string[] {
        if (this.isRemoteScenario()) {
            return this.getRemoteSourcePaths();
        }
        return this.getLocalSourcePaths();
    }


    public static async setAsset<T>(pref: string, newValue: T, raiseError: boolean = true): Promise<void> {
        try {
            // Updates workspace settings
            // eslint-disable-next-line @typescript-eslint/no-floating-promises
            await vs.workspace.getConfiguration().update(pref, newValue);
        } catch (err) {
            if (raiseError) {
                throw new Error(`Preference ${pref} can't be updated on  ${newValue}: ${err}`);
            }
        }
    }

    public static async setGlobalAsset<T>(pref: string, newValue: T, raiseError = true): Promise<void> {
        try {
            await vs.workspace.getConfiguration().update(pref, newValue, vs.ConfigurationTarget.Global);
            await vs.workspace.getConfiguration().update(pref, newValue);
        } catch (err) {
            if (raiseError) {
                throw new Error(`Preference ${pref} can't be updated on  ${newValue}: ${err}`);
            }
        }
    }


    public static getAssetBase<T>(pref: string, defaultValue: T, raiseError: boolean = true): T {
        const asset: T | undefined = vs.workspace.getConfiguration().get(pref);
        if (asset === undefined) {
            if (raiseError) {
                throw new Error(`Incorrect preference: ${pref}`);
            }
            return defaultValue;
        }
        return asset;
    }

    public static getGlobalAssetBase<T>(pref: string, defaultValue: T, raiseError: boolean = true): T {
        const globalValue: T | undefined = vs.workspace.getConfiguration().inspect(pref)?.globalValue as T;
        if (globalValue === undefined) {
            if (raiseError) {
                throw new Error(`Incorrect preference: ${pref}`);
            }
            return defaultValue;
        }
        return globalValue;
    }

    public static getAsset(pref: string): string {
        return this.getAssetBase(pref, "").toString().trim();
    }

    public static getGlobalAsset(pref: string, defaultValue: string): string {
        return this.getGlobalAssetBase(pref, defaultValue, false).toString().trim();
    }

    public static async setHost(host: string): Promise<void> {
        await this.setGlobalAsset(Prefs.HOST_PREF, host, false);
    }

    public static getGlobalHost(): string {
        return this.getGlobalAsset(Prefs.HOST_PREF, defcfg.DefaultConfigValues.DEFAULT_HOST);
    }

    public static getHost(): string {
        return this.getAsset(Prefs.HOST_PREF);
    }

    public static async setPort(port: number): Promise<void> {
        await this.setGlobalAsset(Prefs.PORT_PREF, port, false);
    }

    public static getGlobalPort(): string {
        return this.getGlobalAsset(Prefs.PORT_PREF, `${defcfg.DefaultConfigValues.DEFAULT_PORT}`);
    }

    public static getPort(): string {
        return this.getAsset(Prefs.PORT_PREF);
    }

    public static async setRemotePath(path: string): Promise<void> {
        await this.setAsset(Prefs.REMOTE_PATH_PREF, path, false);
    }

    public static getRemotePath(): string {
        return this.getAsset(Prefs.REMOTE_PATH_PREF);
    }

    public static async setBuildDirectory(path: string): Promise<void> {
        await this.setAsset(Prefs.BUILD_DIR_PREF, path);
    }

    public static async setCmakeOptions(options: string): Promise<void> {
        const cmakeOptions = options.split("\n");
        await this.setAsset(Prefs.CMAKE_OPTIONS_PREF, cmakeOptions);
    }

    public static getBuildDirectory(): string {
        return this.getAsset(Prefs.BUILD_DIR_PREF);
    }

    public static isVerboseTestModeSet(): boolean {
        return this.getAssetBase(Prefs.VERBOSE_MODE_PREF, true);
    }

    public static useStubs(): boolean {
        return this.getAssetBase(Prefs.USE_STUBS_PREF, true);
    }

    public static genSarif(): boolean {
        return this.getAssetBase(Prefs.GEN_SARIF_PREF, true);
    }

    public static timeoutPerFunction(): number {
        return this.getAssetBase(Prefs.FUNC_TIMEOUT_PREF, 30);
    }

    public static timeoutPerTest(): number {
        return this.getAssetBase(Prefs.TEST_TIMEOUT_PREF, 0);
    }

    public static useDeterministicSearcher(): boolean {
        return this.getAssetBase(Prefs.DETERMINISTIC_SEARCHER_PREF, false);
    }

    public static async setVerboseTestMode(mode: boolean): Promise<void> {
        await this.setAsset(Prefs.VERBOSE_MODE_PREF, mode);
    }

    public static generateForStaticFunctions(): boolean {
        return this.getAssetBase(Prefs.STATIC_FUNCTIONS_PREF, true);
    }

    public static showTestResults(): boolean {
        return this.getAssetBase(Prefs.SHOW_TEST_RESULTS_PREF, true);
    }
}
