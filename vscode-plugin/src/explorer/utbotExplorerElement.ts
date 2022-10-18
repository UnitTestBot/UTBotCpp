import * as pathlib from 'path';
import * as vs from 'vscode';
import * as vsUtils from '../utils/vscodeUtils';

export type UTBotElement = UTBotExplorerFolder | UTBotExplorerFile;

export class UTBotExplorerFolder extends vs.TreeItem {

    private static UTBOT_FOLDER_USED = 'utbotfolder_used';
    private static UTBOT_FOLDER_NOT_USED = 'utbotfolder_unused';

    constructor(
        public readonly label: string,
        public readonly path: string,
        public readonly isUsed: boolean,
        public readonly collapsibleState: vs.TreeItemCollapsibleState,

    ) {
        super(label, collapsibleState);
        this.tooltip = `${this.path}`;
        this.description = pathlib.relative(vsUtils.getProjectDirByOpenedFile().fsPath, vs.Uri.file(this.path).fsPath);
    }

    contextValue = this.isUsed ? UTBotExplorerFolder.UTBOT_FOLDER_USED : UTBotExplorerFolder.UTBOT_FOLDER_NOT_USED;
}

export class UTBotExplorerFile extends vs.TreeItem {
    private static UTBOT_FILE = 'utbotfile';
    constructor(
        public readonly label: string,
        public readonly path: string,
        public readonly collapsibleState: vs.TreeItemCollapsibleState,

    ) {
        super(label, collapsibleState);
        this.tooltip = `${this.path}`;
        this.description = pathlib.relative(vsUtils.getProjectDirByOpenedFile().fsPath, this.path);
    }

    contextValue = UTBotExplorerFile.UTBOT_FILE;
}
