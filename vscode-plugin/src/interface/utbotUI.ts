import * as vs from 'vscode';
import { Commands } from '../config/commands';
import { Prefs } from '../config/prefs';
import { UTBotProjectTarget } from '../explorer/UTBotProjectTarget';
import { Progress } from '../requests/params';

export namespace utbotUI {
 
    export namespace titles {
        export const FIVE_RULES_ON = '$(check) UTBot: Verbose Formatting';
        export const FIVE_RULES_OFF = '$(chrome-close) UTBot: Verbose Formatting';
        export const CONNECTION_ALIVE = '$(remote) UTBot: Server Connection';
        export const CONNECTION_DEAD = '$(error) UTBot: Server Connection';
        export const CONNECTION_SYNC = '$(sync~spin) UTBot: Server Connection';
        export const LOG_SETTINGS = '$(book) UTBot: Log Settings';
        export const SERVER_LOG = 'UTBot: Server Log';
        export const CLIENT_LOG = 'UTBot: Client Log';
        export const GTEST_OUT = 'UTBot: Test Console';
    }

    export enum Priority { 
        LOW = 99998, 
        MEDIUM, 
        HIGH 
    }

    export class Indicators {
        public readonly logSettingsStatusBarItem: vs.StatusBarItem;
        public readonly verboseTestsStatusBarItem: vs.StatusBarItem;
        public readonly connectionStatusBarItem: vs.StatusBarItem;
        
        private static _indicators: Indicators;

        private constructor() {
            this.logSettingsStatusBarItem = vs.window.createStatusBarItem(undefined, Priority.LOW);
            this.verboseTestsStatusBarItem = vs.window.createStatusBarItem(undefined, Priority.MEDIUM);
            this.connectionStatusBarItem = vs.window.createStatusBarItem(undefined, Priority.HIGH);
            this.setupCommands();
            this.setupTitles();
        }

        setupCommands(): void {
            this.verboseTestsStatusBarItem.command = Commands.UpdateVerboseTestFlag;
            this.logSettingsStatusBarItem.command = Commands.SelectLoggingLevel;
        }

        setupTitles(): void {
            this.logSettingsStatusBarItem.text = utbotUI.titles.LOG_SETTINGS;
            this.verboseTestsStatusBarItem.text = Prefs.isVerboseTestModeSet() ? 
                                                    utbotUI.titles.FIVE_RULES_ON : 
                                                    utbotUI.titles.FIVE_RULES_OFF;
            this.connectionStatusBarItem.text = utbotUI.titles.CONNECTION_SYNC;
        }

        static get indicators(): Indicators {
            if (!Indicators._indicators) {
                this._indicators = new Indicators();
            }

            return this._indicators;
        }

        public showAll(): void {
            this.logSettingsStatusBarItem.show();
            this.connectionStatusBarItem.show();
            this.verboseTestsStatusBarItem.show();
        }
    }

    export class Channels {
        public readonly outputClientLogChannel: vs.OutputChannel;
        public readonly outputServerLogChannel: vs.OutputChannel;
        public readonly outputGTestChannel: vs.OutputChannel;
        
        private static _channels: Channels; 

        private constructor() {
            this.outputServerLogChannel = vs.window.createOutputChannel(titles.SERVER_LOG);
            this.outputClientLogChannel = vs.window.createOutputChannel(titles.CLIENT_LOG);
            this.outputGTestChannel = vs.window.createOutputChannel(titles.GTEST_OUT);
        }

        static get channels(): Channels {
            if (!Channels._channels) {
                this._channels = new Channels();
            }

            return this._channels;
        }
    }

    export const channels = (): Channels => {
        return Channels.channels;
    };

    export const indicators = (): Indicators => {
        return Indicators.indicators;
    };

    export class ProgressKey {
        constructor(public readonly value: number) { }
    }

    export class ProgressState {
        message: string | undefined = undefined;
        percent: number = 0;
    }
    export class Progresses {
        private static _progresses: Progresses; 

        private constructor() {
            
        }

        static get progresses(): Progresses {
            if (!Progresses._progresses) {
                this._progresses = new Progresses();
            }

            return this._progresses;
        }
        
        private value = 0;
        private map: Map<ProgressKey, [Progress, ProgressState]> = new Map();

        private generateProgressKey(): ProgressKey {
            return new ProgressKey(this.value++);
        }
        
        private registerProgress(progress: Progress): ProgressKey {
            const key = this.generateProgressKey();
            this.map.set(key, [progress, new ProgressState()]);
            return key;
        }

        private unregisterProgress(progressKey: ProgressKey): void {
            this.map.delete(progressKey);
        }

        private get(key: ProgressKey): [Progress, ProgressState] {
            // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
            return this.map.get(key)!;
        }

        public report(progressKey: ProgressKey, message: string, increment: number = 0): void {
            const [progress, state] = this.get(progressKey);
            if (message !== state.message) {
                progress.report({ increment: -state.percent });
                state.percent = 0;
            }
            progress.report({
                increment: increment,
                message: message
            });
            state.message = message;
            state.percent += increment;
        }

        public async withProgress<R>(
            task: (progressKey: ProgressKey, token: vs.CancellationToken) => Promise<R>): Promise<R> {
            const progressTask = async (progress: Progress, token: vs.CancellationToken): Promise<R> => {
                const progressKey = this.registerProgress(progress);
                const value = await task(progressKey, token);
                this.unregisterProgress(progressKey);
                return value;
            };
            const options = {
                location: vs.ProgressLocation.Notification,
                cancellable: true
            };
            return Promise.resolve(vs.window.withProgress(options, progressTask));
        }
    }

    export const progresses = (): Progresses => {
        return Progresses.progresses;
    };

    export class TargetQuickPickItem implements vs.QuickPickItem {
        label: string;
        description: string;
        detail: string;
        
        constructor(readonly target: UTBotProjectTarget) {
            this.label = target.name;
            this.detail = target.path;
            this.description = target.description;
        }
    }
}
