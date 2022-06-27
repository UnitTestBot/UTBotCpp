import * as grpc from 'grpc';
import * as os from 'os';
import * as vs from 'vscode';
import * as defcfg from '../config/defaultValues';
import * as messages from '../config/notificationMessages';
import { Prefs } from '../config/prefs';
import { ExtensionLogger } from '../logger';
import { TestsGenServiceClient } from '../proto-ts/testgen_grpc_pb';
import { GrpcServicePing } from '../utils/grpcServicePing';
import * as pathUtils from '../utils/pathUtils';
import { WizardEventsEmitter } from './wizardEventsEmitter';
import { WizardHtmlBuilder } from './wizardHtmlBuilder';
const { logger } = ExtensionLogger;

export class UtbotWizardPanel {

    public static currentPanel: UtbotWizardPanel | undefined;
    private disposables: vs.Disposable[] = [];
    private static PING_TIMEOUT_MS = 5000;
    private static events = WizardEventsEmitter.getWizardEventsEmitter();

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
                    case 'run_installator':
                        this.runInstallationScript();
                        break;
                    case 'set_host_and_port':
                        await Prefs.setHost(message.host);
                        await Prefs.setPort(message.port);
                        break;
                    case 'set_mapping_path':
                        await Prefs.setRemotePath(message.mappingPath);
                        break;
                    case 'set_build_info':
                        await Prefs.setBuildDirectory(message.buildDirectory);
                        await Prefs.setCmakeOptions(message.cmakeOptions);
                        break;
                    case 'test_connection':
                        this.pingAction(
                            message.host,
                            message.port,
                            'test_server_ping_success',
                            'test_server_ping_failure'
                        );
                        break;
                    case 'check_connection':
                        this.pingAction(
                            message.host,
                            message.port,
                            'check_server_ping_success',
                            'check_server_ping_failure'
                        );
                        break;
                    case 'close_wizard':
                        this.panel.dispose();
                        break;
                    default:
                        messages.showErrorMessage(`Unknown message (${message.command}) from WizardWebView: ${message}`);
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

    private pingAction(host: string, port: number, successCmd: string, failureCmd: string): void {
        const servicePing = new GrpcServicePing(
            new TestsGenServiceClient(
                `${host}:${port}`,
                grpc.credentials.createInsecure()
            )
        );
        servicePing.ping(UtbotWizardPanel.PING_TIMEOUT_MS).then(async pinged => {
            if (pinged) {
                await this.panel.webview.postMessage({ command: successCmd });
            } else {
                await this.panel.webview.postMessage({ command: failureCmd });
            }
        }).catch(async err => {
            logger.error(`Error! ${err}`);
            await this.panel.webview.postMessage({ command: failureCmd });
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
        const stylesUri = webview.asWebviewUri(vs.Uri.joinPath(this.context.extensionUri, 'media', 'wizard.css'));
        const vscodeUri = webview.asWebviewUri(vs.Uri.joinPath(this.context.extensionUri, 'media', 'vscode.css'));
        const scriptUri = webview.asWebviewUri(vs.Uri.joinPath(this.context.extensionUri, 'media', 'wizard.js'));
        const htmlUri = vs.Uri.joinPath(this.context.extensionUri, 'media', 'wizard.html');

        const predictedBuildDirectory = await defcfg.DefaultConfigValues.getDefaultBuildDirectoryPath();

        const htmlBuilder = new WizardHtmlBuilder(htmlUri);
        return htmlBuilder
            .setVSCodeStyleUri(vscodeUri)
            .setCustomStyleUri(stylesUri)
            .setScriptUri(scriptUri)
            .setPredictedHost(defcfg.DefaultConfigValues.getDefaultHost())
            .setPredictedPort(defcfg.DefaultConfigValues.getDefaultPort())
            .setPredictedRemotePath(defcfg.DefaultConfigValues.getDefaultRemotePath())
            .setPredictedBuildDirectory(predictedBuildDirectory)
            .setPlatform(os.platform())
            .setCmakeOptions(defcfg.DefaultConfigValues.DEFAULT_CMAKE_OPTIONS)
            .build();
    }
}
