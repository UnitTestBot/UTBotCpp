import * as vs from 'vscode';
import { Commands } from '../config/commands';
import { DefaultConfigValues } from "../config/defaultValues";
import { Prefs } from '../config/prefs';
import * as pathUtils from '../utils/pathUtils';
import { registerCommand } from '../utils/utils';
import { UTBotElement, UTBotExplorerFile, UTBotExplorerFolder } from "./utbotExplorerElement";
import { UtbotExplorerEventsEmitter } from "./utbotExplorerEventsEmitter";
import { UTBotExplorerStateStorage } from "./utbotExplorerStateStorage";
import { UTBotExplorerTargetElement } from './UTBotExplorerTargetElement';
import { UTBotSourceFoldersProvider } from "./utbotFoldersProvider";
import { UTBotFoldersStorage } from "./utbotFoldersStorage";
import { UTBotTargetsProvider } from './UTBotTargetsProvider';

export class UTBotExplorer {

    private targetsViewer: vs.TreeView<UTBotExplorerTargetElement>;
    private sourceFoldersViewer: vs.TreeView<UTBotElement>;

    constructor(context: vs.ExtensionContext) {
        // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
        const rootPath = pathUtils.getRootPath()!;

        const targetsProvider = new UTBotTargetsProvider(rootPath, context);
        this.targetsViewer = vs.window.createTreeView('UTBotTargets', {
            treeDataProvider: targetsProvider,
        });

        vs.commands.registerCommand(Commands.RefreshUTBotTargetsView, async () => await targetsProvider.requestRefresh());

        vs.commands.registerCommand(Commands.AddUTBotTarget, (target: UTBotExplorerTargetElement) => {
            targetsProvider.setTarget(target);
        });

        vs.commands.registerCommand(Commands.AddUTBotTargetPath, (targetPath: string) => {
            targetsProvider.setPrimaryTargetPath(targetPath);
        });

        UtbotExplorerEventsEmitter.getUbotExplorerEventsEmitter().onDidAddTargetsEventEmitter.on((utbotProjectTargetsInfo) => {
            targetsProvider.refresh(utbotProjectTargetsInfo.targets);
            if (utbotProjectTargetsInfo.priorityTarget) {
                targetsProvider.setPrimaryTargetPath(utbotProjectTargetsInfo.priorityTarget?.path);
            }
        });


        const sourceFoldersProvider = new UTBotSourceFoldersProvider(rootPath, context);
        this.sourceFoldersViewer = vs.window.createTreeView('UTBotFolders', {
            treeDataProvider: sourceFoldersProvider,
        });

        const events = UtbotExplorerEventsEmitter.getUbotExplorerEventsEmitter();

        registerCommand(Commands.RefreshUTBotSourceFoldersView, () => sourceFoldersProvider.refresh());

        registerCommand(Commands.AddUTBotSourceFolder, (folder: UTBotExplorerFolder) => {
            void setSourceDirectoryByPath(folder.path);
        });

        registerCommand(Commands.DeleteUTBotSourceFolder, (folder: UTBotExplorerFolder) => {
            void unsetSourceDirectoryByPath(folder.path);
        });

        vs.workspace.onDidChangeConfiguration((event) => {
            if (event.affectsConfiguration(Prefs.SOURCE_DIRS_PREF)) {
                UTBotFoldersStorage.instance.setFoldersFromConfiguration();
                sourceFoldersProvider.refresh();
            }
        });

        vs.workspace.onDidCreateFiles((_event) => {
            sourceFoldersProvider.refresh();
        });

        vs.workspace.onDidDeleteFiles((_event) => {
            sourceFoldersProvider.refresh();
        });

        vs.workspace.onDidRenameFiles((_event) => {
            sourceFoldersProvider.refresh();
        });

        vs.workspace.onDidChangeWorkspaceFolders((_event) => {
            sourceFoldersProvider.refresh();
        });

        events.onDidRequestSetDefaultSourceFoldersEventsEmmiter.on(async () => {
            void setInitSourceDirectories();
        });

        this.sourceFoldersViewer.onDidCollapseElement((event) => {
            UTBotExplorerStateStorage.getInstance().saveState(event.element.path, vs.TreeItemCollapsibleState.Collapsed);
            sourceFoldersProvider.refresh();
        });

        this.sourceFoldersViewer.onDidExpandElement((event) => {
            UTBotExplorerStateStorage.getInstance().saveState(event.element.path, vs.TreeItemCollapsibleState.Expanded);
            sourceFoldersProvider.refresh();
        });

        this.sourceFoldersViewer.onDidChangeSelection(async (event) => {
            const paths =
                event.selection
                    .filter(element => element instanceof UTBotExplorerFile)
                    .map(element => element.path);

            if (paths.length > 0) {
                await vs.workspace.openTextDocument(vs.Uri.file(paths[0]))
                    .then(async (doc) => {
                        await vs.window.showTextDocument(doc, { preview: false });
                    });
            }
        });


        async function setInitSourceDirectories(): Promise<void> {
            // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
            const rootPath = pathUtils.getRootPath();
            if (!rootPath) {
                return;
            }
            const initSourceDirectoriesNames: Array<string> = [];
            await clearSourceDirectories();
            await vs.workspace.fs.readDirectory(vs.Uri.file(rootPath))
                .then(resultArray => {
                    resultArray.forEach(([name, type]) => {
                        // add only non hidden directories and not a build directory by default
                        if (type === vs.FileType.Directory
                            && !name.startsWith('.')
                            && !DefaultConfigValues.looksLikeBuildDirectory(name)
                            && !DefaultConfigValues.looksLikeTestDirectory(name)) {
                            initSourceDirectoriesNames.push(pathUtils.fsJoin(rootPath, name));
                        }
                    });
                });
            await setSourceDirectoryNonRecursive(rootPath);
            initSourceDirectoriesNames.forEach(async path => await setSourceDirectoryByPath(path));
        }

        async function setSourceDirectoryNonRecursive(path: string): Promise<void> {
            sourceFoldersProvider.markAsUsedNonRecursive(path);
            await UTBotFoldersStorage.instance.updateConfigurationFromCurrentState();
            sourceFoldersProvider.refresh();
        }

        async function setSourceDirectoryByPath(path: string): Promise<void> {
            sourceFoldersProvider.markAsUsed(path);
            await UTBotFoldersStorage.instance.updateConfigurationFromCurrentState();
            sourceFoldersProvider.refresh();
        }

        async function unsetSourceDirectoryByPath(path: string): Promise<void> {
            sourceFoldersProvider.marksAsNotUsed(path);
            await UTBotFoldersStorage.instance.updateConfigurationFromCurrentState();
            sourceFoldersProvider.refresh();
        }

        async function clearSourceDirectories(): Promise<void> {
            await Prefs.setAsset(Prefs.SOURCE_DIRS_PREF, "");
            // explicitly update storage
            UTBotFoldersStorage.instance.setFoldersFromConfiguration();
        }
    }
}
