import * as grpc from 'grpc';
import * as os from 'os';
import * as vs from 'vscode';
import {Uri} from 'vscode';
import * as defcfg from '../config/defaultValues';
import * as messages from '../config/notificationMessages';
import {Prefs} from '../config/prefs';
import {ExtensionLogger} from '../logger';
import {TestsGenServiceClient} from '../proto-ts/testgen_grpc_pb';
import {GrpcServicePing} from '../utils/grpcServicePing';
import * as pathUtils from '../utils/pathUtils';
import {WizardEventsEmitter} from './wizardEventsEmitter';
import * as fs from "fs";
import * as vsUtils from "../utils/vscodeUtils";

const { logger } = ExtensionLogger;

function getNonce(): string {
    let text = '';
    const possible = 'abcdefghijklmnopqrstuvwxyz0123456789';
    for (let i = 0; i < 10; i++) {
        text += possible.charAt(Math.floor(Math.random() * possible.length));
    }
    return text;
}

export class UtbotWizardPanel {

    public static currentPanel: UtbotWizardPanel | undefined;
    private disposables: vs.Disposable[] = [];
    private static PING_TIMEOUT_MS = 5000;
    private static events = WizardEventsEmitter.getWizardEventsEmitter();
    private static checkNonInitializedVars = /[{][{][a-zA-Z]+[}][}]/g;

    public static createOrShow(context: vs.ExtensionContext): void {
        const column = vs.window.activeTextEditor
            ? vs.window.activeTextEditor.viewColumn
            : undefined;

        if (UtbotWizardPanel.currentPanel) {
            UtbotWizardPanel.currentPanel.panel.reveal(column);
            return;
        }

        const panel = vs.window.createWebviewPanel(
            UtbotWizardPanel.getIdentifier(),
            UtbotWizardPanel.getTitle(),
            column || UtbotWizardPanel.getEditorColumnStyle(),
            UtbotWizardPanel.getOptions(context.extensionUri),
        );

        UtbotWizardPanel.currentPanel = new UtbotWizardPanel(panel, context);
    }

    constructor(private readonly panel: vs.WebviewPanel, private readonly context: vs.ExtensionContext) {
        void this.update();
        this.panel.onDidDispose(() => this.dispose(), null, this.disposables);

        this.panel.onDidChangeViewState(
            _event => {
                if (this.panel.visible) {
                    void this.update();
                }
            },
            null,
            this.disposables
        );

        this.panel.webview.onDidReceiveMessage(
            async message => {
                switch (message.command) {
                    case 'openSFTPSettings':
                        if (message.key !== undefined) {
                            const keyName = message.key;
                            //await vs.commands.executeCommand('workbench.action.openSettings', 'Natizyskunk.sftp.remotePath');
                            await vs.commands.getCommands(true).then(
                                async (commands: string[]) => {
                                    if (commands.includes("sftp.config")) {
                                        await vs.commands.executeCommand("sftp.config").then(
                                            async () => {
                                                // TODO: positioning at keyname/value line
                                                // await vs.commands.executeCommand('actions.find', `${keyName}`).then(
                                                //     () => messages.showErrorMessage(`query: @${keyName}`)
                                                // );
                                            }
                                        );
                                    } else {
                                        messages.showErrorMessage("SFTP plugin isn't installed!");
                                    }
                                }
                            );
                        }
                        break;
                    case 'run_installer':
                        this.runInstallationScript();
                        break;
                    case 'set_server_setup':
                        await Prefs.setHost(message.host);
                        await Prefs.setPort(message.port);
                        await Prefs.setRemotePath(message.mappingPath);
                        break;
                    case 'set_build_info':
                        await Prefs.setBuildDirectory(message.buildDirectory);
                        await Prefs.setCmakeOptions(message.cmakeOptions);
                        break;
                    case 'check_connection':
                    case 'test_connection':
                        this.pingAction(
                            message.host,
                            message.port,
                            message.command + '_success',
                            message.command + '_failure');
                        break;
                    case 'close_wizard':
                        this.panel.dispose();
                        break;
                    case 'dbg_message':
                        logger.info(`dbg_message: ${message.message}`);
                        break;
                    default:
                        messages.showErrorMessage(`Unknown message (${message.command}) from WizardWebView: ${message}`);
                        break;
                }
            },
            null,
            this.disposables
        );
    }

    private runInstallationScript(): void {
        const onDiskPath = vs.Uri.file(pathUtils.fsJoin(this.context.extensionPath, 'scripts', 'utbot_installer.sh'));
        const terminal = vs.window.createTerminal('UTBot Server Installation');
        terminal.show();
        terminal.sendText(onDiskPath.fsPath, true);
    }

    private lastRequest: Promise<string|null> | null = null;
    private pingAction(host: string, port: number, successCmd: string, failureCmd: string): void {
        const servicePing = new GrpcServicePing(
            this.context.extension.packageJSON.version,
            new TestsGenServiceClient(
                `${host}:${port}`,
                grpc.credentials.createInsecure()
            )
        );

        const capturedPingPromiseForLambda = servicePing.ping(UtbotWizardPanel.PING_TIMEOUT_MS);
        this.lastRequest = capturedPingPromiseForLambda;
        capturedPingPromiseForLambda.then(async serverVer => {
            if (this.lastRequest !== capturedPingPromiseForLambda) {
                // ignore: there is more actual request
                return;
            }
            if (serverVer !== null) {
                await this.panel.webview.postMessage({
                    command: successCmd,
                    clientVersion: this.context.extension.packageJSON.version,
                    serverVersion: (serverVer.length === 0
                       ? "undefined"
                       : serverVer)});
            } else {
                await this.panel.webview.postMessage({ command: failureCmd });
            }
        }).catch(async err => {
            logger.error(`Error! ${err}`);
            if (this.lastRequest === capturedPingPromiseForLambda) {
                await this.panel.webview.postMessage({command: failureCmd});
            }
        });
        return;
    }

    public async update(): Promise<void> {
        this.panel.webview.html = await this.getHtmlForWebview(this.panel.webview);
    }

    public dispose(): void {
        UtbotWizardPanel.currentPanel = undefined;

        // Clean up our resources
        this.panel.dispose();

        while (this.disposables.length) {
            const x = this.disposables.pop();
            if (x) {
                x.dispose();
            }
        }

        void UtbotWizardPanel.events.onDidCloseWizardEventEmitter.fire();
    }

    public static getIdentifier(): string {
        return 'utbotQuickstartWizard';
    }

    public static getTitle(): string {
        return 'UTBot: Quickstart Wizard';
    }

    public static getEditorColumnStyle(): vs.ViewColumn {
        return vs.ViewColumn.One;
    }

    public static getOptions(extensionUri: vs.Uri): vs.WebviewPanelOptions & vs.WebviewOptions {
        return {
            enableScripts: true,
            retainContextWhenHidden: true,
            localResourceRoots: [
                vs.Uri.file(pathUtils.fsJoin(extensionUri.fsPath, 'media')),
                vs.Uri.file(pathUtils.fsJoin(extensionUri.fsPath, 'scripts'))
            ]
        };
    }

    private async getHtmlForWebview(webview: vs.Webview): Promise<string> {

        const extUri = this.context.extensionUri;
        function mediaPath(fileName: string): Uri {
            return webview.asWebviewUri(vs.Uri.joinPath(extUri, 'media', fileName));
        }

        const predictedBuildDirectory = await defcfg.DefaultConfigValues.getDefaultBuildDirectoryPath();
        const initVars: {param: string; value: string|undefined}[] = [
            // UI javascript
            {param: 'scriptUri', value: mediaPath('wizard.js').toString()},

            // Security (switched off)
            //{param: 'wvscriptUri', value: webview.cspSource},
            //{param: 'nonce', value: getNonce()}, // Use a nonce to only allow a specific script to be run.

            // CSS in header
            {param: 'vscodeUri', value: mediaPath('vscode.css').toString()},
            {param: 'stylesUri', value: mediaPath('wizard.css').toString()},

            // vars
            {param: 'os', value: os.platform()},
            {param: 'projectDir', value: defcfg.DefaultConfigValues.toWSLPathOnWindows(vsUtils.getProjectDirByOpenedFile().fsPath)},
            {param: 'defaultPort', value: defcfg.DefaultConfigValues.DEFAULT_PORT.toString()},
            {param: 'sftpHost', value: vsUtils.getFromSftpConfig("host")},
            {param: 'sftpDir', value: vsUtils.getFromSftpConfig("remotePath")},

            // connection tab
            {param: 'predictedHost', value: defcfg.DefaultConfigValues.getDefaultHost()},
            {param: 'predictedPort', value: Prefs.getGlobalPort()},
            {param: 'predictedRemotePath', value: defcfg.DefaultConfigValues.getDefaultRemotePath()},

            // project tab
            {param: 'predictedBuildDirectory', value:predictedBuildDirectory},
            {param: 'cmakeOptions', value: defcfg.DefaultConfigValues.DEFAULT_CMAKE_OPTIONS.join("\n")},
        ];

        let content = fs.readFileSync(mediaPath('wizard.html').fsPath, 'utf8');
        for (const p2v of initVars) {
            logger.info(`map ${p2v.param} -> ${p2v.value}`);
            // eslint-disable-next-line @typescript-eslint/ban-ts-ignore
            // @ts-ignore
            content = content.replaceAll(`{{${p2v.param}}}`,
                `${p2v.value === undefined
                ? '' 
                : p2v.value}`);
        }

        const uninitVars = UtbotWizardPanel.checkNonInitializedVars.exec(content);
        if (uninitVars !== null) {
            logger.error(`Error! Wizard has non-initialized variable ${uninitVars[0]}.`);
            throw new Error(`Wizard has non-initialized variable ${uninitVars[0]}.`);
        }
        return content;
    }
}
