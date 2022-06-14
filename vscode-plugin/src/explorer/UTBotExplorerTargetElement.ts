import * as vs from 'vscode';
import { UTBotTargetsStorage } from './utbotTargetsStorage';
import { IconPaths } from '../interface/iconPaths';
import { Commands } from '../config/commands';

export class UTBotExplorerTargetElement extends vs.TreeItem {
    private static UTBOT_TARGET_UNUSED = 'utbottarget_unused';
    private static UTBOT_TARGET_USED = 'utbottarget_used';
    constructor(
        public readonly label: string,
        public readonly path: string,
        public readonly description: string,
        private iconPaths: IconPaths
    ) {
        super(label, vs.TreeItemCollapsibleState.None);
        this.tooltip = `${this.path}`;
        this.description = description;
        this.command = { title: "Mark as UTBot Target", command: Commands.AddUTBotTarget, arguments: [this] };
        
        if (this.path === UTBotTargetsStorage.instance.primaryTargetPath) {
            this.contextValue = UTBotExplorerTargetElement.UTBOT_TARGET_USED;
            this.iconPath = this.iconPaths.targetUsed;
        } else {
            this.contextValue = UTBotExplorerTargetElement.UTBOT_TARGET_UNUSED;
            this.iconPath = this.iconPaths.targetUnused;
        }
    }
}
