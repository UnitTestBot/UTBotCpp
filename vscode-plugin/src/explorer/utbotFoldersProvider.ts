/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as pathlib from 'path';
import * as vs from 'vscode';
import { showInfoMessage } from '../config/notificationMessages';
import { IconPaths } from '../interface/iconPaths';
import { FSUtils } from '../utils/fsUtils';
import { UTBotElement, UTBotExplorerFile, UTBotExplorerFolder } from './utbotExplorerElement';
import { UTBotExplorerStateStorage } from './utbotExplorerStateStorage';
import { UTBotFoldersStorage } from './utbotFoldersStorage';


export class UTBotSourceFoldersProvider implements vs.TreeDataProvider<UTBotElement> {
    private iconPaths: IconPaths;

    constructor(private workspaceRoot: string, context: vs.ExtensionContext) {
        this.iconPaths = new IconPaths(context);
    }

    private _onDidChangeTreeData: vs.EventEmitter<UTBotElement | undefined | null | void> = new vs.EventEmitter<UTBotElement | undefined | null | void>();
    readonly onDidChangeTreeData: vs.Event<UTBotElement | undefined | null | void> = this._onDidChangeTreeData.event;

    refresh(): void {
        this._onDidChangeTreeData.fire();
    }

    public markAsUsedNonRecursive(folderPath: string): void {
        UTBotFoldersStorage.instance.addFolder(folderPath);
    }

    public markAsUsed(folderPath: string): void {
        UTBotFoldersStorage.instance.addFolderRecursive(folderPath);
    }

    public marksAsNotUsed(folderPath: string): void {
        UTBotFoldersStorage.instance.removeFolderRecursive(folderPath);
    }

    getTreeItem(element: UTBotElement): vs.TreeItem {
        return element;
    }

    getChildren(element?: UTBotElement): UTBotElement[] {
        if (!this.workspaceRoot) {
            showInfoMessage('No folders in empty workspace');
            return [];
        }

        const folderPathHandler = (path: string): UTBotExplorerFolder => {
            const isUsed = UTBotFoldersStorage.instance.containsFolder(path);
            let state = UTBotExplorerStateStorage.getInstance().getSavedState(path);
            if (state === undefined) {
                state = vs.TreeItemCollapsibleState.Collapsed;
            }
            if (path === this.workspaceRoot) {
                state = vs.TreeItemCollapsibleState.Expanded;
            }
            return this.getUTBotSourceFolder(path, isUsed, state);
        };

        const filePathHandler = (path: string): UTBotExplorerFile => {
            return this.getUTBotExplorerFile(path);
        };

        const foldersAndFilesHandler = (foldersFiles: [string[], string[]]): UTBotElement[] => {
            const folderElements = foldersFiles[0].map(folderPathHandler);
            const fileElements = foldersFiles[1].map(filePathHandler);
            const children: UTBotElement[] = [];
            for (const folderElement of folderElements) {
                children.push(folderElement);
            }
            for (const fileElement of fileElements) {
                children.push(fileElement);
            }
            return children;
        };

        if (element) {
            if (element instanceof UTBotExplorerFile) {
                return [];
            }
            return foldersAndFilesHandler(this.getSubFolders(element.path));
        } else {
            return foldersAndFilesHandler([[this.workspaceRoot], []]);
        }
    }

    private getSubFolders(path: string): [string[], string[]] {
        const directories = FSUtils.getDirectories(path);
        const files = FSUtils.getFiles(path);
        const result: [string[], string[]] = [directories, files];
        return result;
    }

    private getUTBotExplorerFile(path: string): UTBotExplorerFile {
        const label = pathlib.basename(path);
        const element = new UTBotExplorerFile(label, path, vs.TreeItemCollapsibleState.None);
        element.iconPath = this.iconPaths.file;
        return element;
    }

    private getUTBotSourceFolder(path: string, isUsed: boolean, state: vs.TreeItemCollapsibleState): UTBotExplorerFolder {
        const label = pathlib.basename(path);
        const element = new UTBotExplorerFolder(label, path, isUsed, state);
        if (isUsed && state === vs.TreeItemCollapsibleState.Collapsed) {
            element.iconPath = this.iconPaths.folderUtbot;
            return element;
        } else if (isUsed && state === vs.TreeItemCollapsibleState.Expanded) {
            element.iconPath = this.iconPaths.folderUtbotOpen;
            return element;
        } else if (!isUsed && state === vs.TreeItemCollapsibleState.Collapsed) {
            element.iconPath = this.iconPaths.folderDefault;
            return element;
        } else {
            element.iconPath = this.iconPaths.folderDefaultOpen;
            return element;
        }

    }
}
