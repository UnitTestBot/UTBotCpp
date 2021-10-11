/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as vs from 'vscode';
import { showInfoMessage } from '../config/notificationMessages';
import { Prefs } from '../config/prefs';
import { IconPaths } from '../interface/iconPaths';
import { UtbotExplorerEventsEmitter } from './utbotExplorerEventsEmitter';
import { UTBotExplorerTargetElement } from './UTBotExplorerTargetElement';
import { UTBotProjectTarget } from './UTBotProjectTarget';
import { UTBotTargetsStorage } from './utbotTargetsStorage';


export class UTBotTargetsProvider implements vs.TreeDataProvider<UTBotExplorerTargetElement> {
    private iconPaths: IconPaths;
    
    constructor(private workspaceRoot: string, private context: vs.ExtensionContext) {
        this.iconPaths = new IconPaths(context);

        const targets = Prefs.getTargets(context);
        const primaryTargetPath = Prefs.getPrimaryTargetPath(context);
        if (targets) {
            this.refresh(targets);
        }
        if (primaryTargetPath) {
            this.setPrimaryTargetPath(primaryTargetPath);
        }
    }

    private _onDidChangeTreeData: vs.EventEmitter<UTBotExplorerTargetElement | undefined | null | void> = new vs.EventEmitter<UTBotExplorerTargetElement | undefined | null | void>();
    readonly onDidChangeTreeData: vs.Event<UTBotExplorerTargetElement | undefined | null | void> = this._onDidChangeTreeData.event;

    refresh(targets: Array<UTBotProjectTarget>): void {
        void Prefs.setTargets(this.context, targets);

        UTBotTargetsStorage.instance.targets = targets;
        this._onDidChangeTreeData.fire();
    }

    requestRefresh(): Promise<void> {
        return UtbotExplorerEventsEmitter.getUbotExplorerEventsEmitter().onDidRequestTargetsEmitter.fire();
    }

    setPrimaryTargetPath(path: string): void {
        void Prefs.setPrimaryTargetPath(this.context, path);

        UTBotTargetsStorage.instance.primaryTargetPath = path;
        this.refresh(UTBotTargetsStorage.instance.targets);
    }

    setTarget(target: UTBotExplorerTargetElement): void {
        this.setPrimaryTargetPath(target.path);
    }

    getTreeItem(element: UTBotExplorerTargetElement): vs.TreeItem {
        return element;
    }

    getChildren(element?: UTBotExplorerTargetElement): UTBotExplorerTargetElement[] {
        if (!this.workspaceRoot) {
            showInfoMessage('No targets in empty workspace');
            return [];
        }

        if (!element) {
            const targets = UTBotTargetsStorage.instance.targets;
            return targets.sort(UTBotProjectTarget.compareFn).map((target) => this.getTargetElement(target));
        } else {
            return [];
        }
    }

    private getTargetElement(target: UTBotProjectTarget): UTBotExplorerTargetElement {
        const element = new UTBotExplorerTargetElement(target.name, target.path, target.description, this.iconPaths);
        return element;
    }
}



