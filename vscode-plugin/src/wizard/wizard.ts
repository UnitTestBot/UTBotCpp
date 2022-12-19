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
import {NodeSSH} from "node-ssh";

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
        this.panel.webview.onDidReceiveMessage(
            async message => {
                switch (message.command) {
                    case 'run_installer':
                        this.runInstallationScript();
                        break;
                    case 'set_server_setup':
                        await Prefs.setHost(message.host);
                        await Prefs.setGRPCPort(message.portGRPC);
                        await Prefs.setRemotePath(message.serverPath);
                        await this.setupSFTP(
                            !!message.portSFTP,
                            message.host,
                            message.portSFTP,
                            message.serverPath);
                        break;
                    case 'set_build_info':
                        await Prefs.setBuildDirectory(message.buildDirectory);
                        await Prefs.setCmakeOptions(message.cmakeOptions);
                        break;
                    case 'close_wizard':
                        this.panel.dispose();
                        break;
                    case 'dbg_message':
                        logger.info(`dbg_message: ${message.message}`);
                        break;
                    case 'check_plugins':
                        //logger.info(`check_plugins`);
                        this.checkPlugins();
                        break;
                    case 'check_sftp_on_start':
                        this.checkSFTPOnStart();
                        break;
                    case 'check_sarif_on_start':
                        this.checkSARIFOnStart();
                        break;
                    default: {
                        if (message.command.endsWith('test_connection')) {
                            //messages.showErrorMessage(`!!!!!! message (${message.command}) from WizardWebView: ${message}`);
                            if (message.command.startsWith(`GRPC_`)) {
                                this.pingAction(
                                    message.host,
                                    message.port,
                                    message.command + '_success',
                                    message.command + '_failure');
                                break;
                            }
                            else if (message.command.startsWith(`SFTP_`)) {
                                this.pingSFTPAction(
                                    message.host,
                                    message.port,
                                    message.command + '_success',
                                    message.command + '_failure');
                                break;
                            }
                        }
                        messages.showErrorMessage(`Unknown message (${message.command}) from WizardWebView: ${message}`);
                        break;
                    }
                }
            },
            null,
            this.disposables
        );
    }

    private checkPlugins(): void  {
        void this.panel.webview.postMessage({
            command: "checkPlugins",
            sftpPluginInstalled: !!vs.extensions.getExtension(messages.defaultSFTP),
            sarifPluginInstalled: !!vs.extensions.getExtension(messages.defaultSARIFViewer)
        });
    }
    private checkSFTPOnStart(): void  {
        messages.showWarningMessage(messages.installSFTP);
    }

    private checkSARIFOnStart(): void  {
        messages.showWarningMessage(messages.installSARIFViewer);
    }

    private static GENERATED = "This file is automatically generated by UnitTestBot.";
    private async setupSFTP(
        activate: boolean,
        host: string,
        portSFTP: string,
        remotePath: string
    ): Promise<void> {
        const sftpExt = vs.extensions.getExtension(messages.defaultSFTP);
        if (!sftpExt) {
            messages.showWarningMessage(messages.installSFTP);
        } else {
            if (!sftpExt.isActive) {
                await sftpExt.activate();
            }
            const workspaceFolderUrl = vs.workspace.workspaceFolders?.[0].uri;
            if (workspaceFolderUrl) {
                const workspaceFolder = workspaceFolderUrl.fsPath;
                const sftpConfigPath = pathUtils.fsJoin(workspaceFolder, '.vscode', 'sftp.json');
                try {
                    // backup config: sftp.json -> sftp.json.old if we have user's version
                    if (fs.existsSync(sftpConfigPath)) {
                        const configContentOld = fs.readFileSync(sftpConfigPath, {encoding:'utf8', flag:'r'});
                        // checks, that the configuration was not created by UTBot
                        if (!configContentOld.includes(UtbotWizardPanel.GENERATED)) {
                            const oldConfigPath = sftpConfigPath + ".old";
                            console.log(`Back up ".vscode/${sftpConfigPath}" to ".vscode/${oldConfigPath}"`);
                            fs.writeFileSync(oldConfigPath, configContentOld);
                        }
                    }

                    if (activate) {
                        const configContent =
`{
    "//comment": "${UtbotWizardPanel.GENERATED}",
    "name": "UTBot Server",
    "host": "${host}",
    "protocol": "sftp",
    "port": ${portSFTP},
    "username": "utbot",
    "password": "utbot",
    "remotePath": "${remotePath}",
    "uploadOnSave": true,
    "useTempFile": false,
    "openSsh": false
}`;
                        if (!fs.existsSync(sftpConfigPath)) {
                           fs.writeFileSync(sftpConfigPath, ' ');
                        }

                        const doc = await vs.workspace.openTextDocument(sftpConfigPath);
                        const editor = await vs.window.showTextDocument(doc, {preview: true, preserveFocus: false});
                        // we need to generate the `onDidSaveTextDocument` event 
                        // it is the only event that is processed by SFTP plugin to change the preload configuration
                        void editor.edit( builder => {
                            builder.delete(new vs.Range(0, 0, 10000, 10000));
                            builder.insert(new vs.Position(0, 0), configContent);
                        })
                        .then( () => {
                            void editor.document.save().then( saved => {
                                if (saved) {
                                    messages.showInfoMessage(`New configuration ".vscode/sftp.json" was saved!`);
                                }
                                void vs.commands.executeCommand('workbench.action.closeActiveEditor');
                                const postponedSync = (): void => {
                                    void vs.commands.executeCommand("sftp.sync.localToRemote", workspaceFolderUrl).then(
                                        () => messages.showInfoMessage(`Project copy was created on UTBot Server at "${remotePath}"`),
                                        (err) => messages.showWarningMessage(`Project copy was not created on UTBot Server at "${remotePath}" with error ` + err)
                                    );
                                };
                                setTimeout(postponedSync, 300);
                            });
                        });
                    } else {
                        fs.unlink(sftpConfigPath, function (err) {
                            if (err) {
                                console.error(err);
                                console.log('File ".vscode/sftp.json" not found');
                            }else{
                                console.log('File ".vscode/sftp.json" was deleted successfully');
                            }
                        });
                    }
                } catch (error) {
                    messages.showWarningMessage("Error while SFTP configuration: "  + error);
                }    
            }
        }
    }

    private runInstallationScript(): void {
        const onDiskPath = vs.Uri.file(pathUtils.fsJoin(this.context.extensionPath, 'scripts', 'utbot_installer.sh'));
        const terminal = vs.window.createTerminal('UTBot Server Installation');
        terminal.show();
        terminal.sendText(onDiskPath.fsPath, true);
    }

    private lastRequest: Promise<any> | null = null;
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
                await this.panel.webview.postMessage({ command: failureCmd});
            }
        }).catch(async err => {
            logger.error(`Error! ${err}`);
            if (this.lastRequest === capturedPingPromiseForLambda) {
                await this.panel.webview.postMessage({command: failureCmd});
            }
        });
        return;
    }

    private lastSFTPRequest: Promise<any> | null = null;
    private pingSFTPAction(host: string, port: number, successCmd: string, failureCmd: string): void {
        const ssh = new NodeSSH();
        const username = defcfg.DefaultConfigValues.getDefaultSFTPUsername().toString();
        const password = defcfg.DefaultConfigValues.getDefaultSFTPPassword().toString();
        const capturedPingPromiseForLambda = ssh.connect({
            host: host,
            port: port,
            username: username,
            password: password
        });
        this.lastSFTPRequest = capturedPingPromiseForLambda;
        capturedPingPromiseForLambda.then( async () => {
            ssh.dispose();
            if (this.lastSFTPRequest !== capturedPingPromiseForLambda) {
                return;
            }
            await this.panel.webview.postMessage({
                command: successCmd,
                clientVersion: "1",
                serverVersion: "1"});
        }, async (error) => {
            ssh.dispose();
            if (this.lastSFTPRequest === capturedPingPromiseForLambda) {
                await this.panel.webview.postMessage({ command: failureCmd});
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
            {param: 'scriptUri', value: mediaPath('wizardHTML.js').toString()},

            // Security (switched off)
            //{param: 'wvscriptUri', value: webview.cspSource},
            //{param: 'nonce', value: getNonce()}, // Use a nonce to only allow a specific script to be run.

            // CSS in header
            {param: 'vscodeUri', value: mediaPath('vscode.css').toString()},
            {param: 'stylesUri', value: mediaPath('wizardHTML.css').toString()},

            // vars
            {param: 'os', value: os.platform()},
            {param: 'projectDir', value: defcfg.DefaultConfigValues.toWSLPathOnWindows(vsUtils.getProjectDirByOpenedFile().fsPath)},
            {param: 'defaultGRPCPort', value: defcfg.DefaultConfigValues.DEFAULT_GRPC_PORT.toString()},
            {param: 'defaultSFTPPort', value: defcfg.DefaultConfigValues.DEFAULT_SFTP_PORT.toString()},
            {param: 'serverHost', value: vsUtils.getFromSftpConfig("host")},
            {param: 'serverDir', value: vsUtils.getFromSftpConfig("remotePath")},

            // connection tab
            {param: 'predictedHost', value: defcfg.DefaultConfigValues.getDefaultHost()},
            {param: 'predictedGRPCPort', value: Prefs.getGlobalGRPCPort()},
            {param: 'predictedSFTPPort', value: defcfg.DefaultConfigValues.getDefaultSFTPPort().toString()},
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
