import * as vs from 'vscode';
import {Client} from '../client/client';
import * as messages from '../config/notificationMessages';
import {Prefs} from '../config/prefs';
import {DummyResponseHandler} from '../responses/responseHandler';
import {utbotUI} from '../interface/utbotUI';
import {ExtensionLogger} from '../logger';
import {ConfigMode, ProjectConfigResponse, ProjectConfigStatus} from '../proto-ts/testgen_pb';
import {ProjectConfigEventsEmitter} from './projectConfigEventsEmitter';

const { logger } = ExtensionLogger;

export class ProjectConfig {
    private static readonly guideUri = "https://github.com/UnitTestBot/UTBotCpp/wiki";

    private readonly projectName: string;
    private readonly projectPath: string;
    private readonly buildDirRelativePath: string;
    private readonly cmakeOptions: Array<string>;

    constructor(private readonly client: Client) {
        this.projectName = Prefs.getProjectName();
        [this.projectPath, this.buildDirRelativePath] = Prefs.getBuildDirPath();
        this.cmakeOptions = Prefs.getCmakeOptions();
    }

    private createBuildDirFailed(resp: string): boolean {
        const message = `Build folder creation failed with the following message:` + resp;
        logger.warn(message);
        messages.showWarningMessage(`${message}. 
                                            Please, follow the [guilde](${ProjectConfig.guideUri}) to configure project.`);
        return false;
    }

    public async configure(configMode: ConfigMode): Promise<boolean> {
        logger.debug('Configure project');

        if (this.client.isConnectionNotEstablished() || this.client.newClient) {
            try {
                await this.client.heartbeat();
            } catch (error) {
                logger.error(`Heartbeat error: ${error}`);
            }
        }

        const response = await this.checkProjectConfiguration(configMode);
        logger.debug(`Received response: ${response}`);
        switch (response.getType()) {
            case ProjectConfigStatus.IS_OK: {
                const message = `Project '${this.projectName}' was successfully configured.`;
                logger.debug(message);
                messages.showInfoMessage(message);
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                await ProjectConfigEventsEmitter.getProjectConfigEventsEmitter().onDidImportProject.fire();
                return true;
            }
            case ProjectConfigStatus.BUILD_DIR_NOT_FOUND: {
                return this.handleBuildDirNotFound();
            }
            case ProjectConfigStatus.COMPILE_COMMANDS_JSON_NOT_FOUND:
            case ProjectConfigStatus.LINK_COMMANDS_JSON_NOT_FOUND: {
                return this.handleJsonFilesNotFound(response.getType());
            }
            case ProjectConfigStatus.BUILD_DIR_CREATION_FAILED: {
                return this.createBuildDirFailed(response.getMessage());
            }
            case ProjectConfigStatus.BUILD_DIR_SAME_AS_PROJECT: { 
                const message = response.getMessage();
                logger.warn(message);
                messages.showWarningMessage(`${message}. 
                                                    Please, follow the [guilde](${ProjectConfig.guideUri}) to configure project.`);
                return false;           
            }
            default: {
                this.handleUnexpectedResponse();
                return false;
            }
        }
    }

    private handleUnexpectedResponse(): void {
        const message = 'Unexpected response from server';
        messages.showErrorMessage(`${message}. Please, try again.`);
    }

    async checkProjectConfiguration(
        configMode: ConfigMode): Promise<ProjectConfigResponse> {
        logger.debug('Check project configuration');

        return utbotUI.progresses().withProgress<ProjectConfigResponse>(async (progressKey, token) => {
            utbotUI.progresses().report(progressKey, "Check project configuration...");
            const responseHandler = new DummyResponseHandler<ProjectConfigResponse>();
            return this.client.checkProjectConfigurationRequest(this.projectName, this.projectPath, this.buildDirRelativePath, this.cmakeOptions, configMode, progressKey, token, responseHandler);
        });
    }

    async handleBuildDirNotFound(): Promise<boolean> {
        logger.info('Build folder not found');

        const yesOption = 'Create build folder';
        return vs.window.showWarningMessage(`Build folder "${this.buildDirRelativePath}"
                specified in [Preferences](command:unittestbot.innercommand.openBuildDirectoryConfig), does not exist.`, ...[yesOption]).then(async selection => {
            if (selection === yesOption) {
                return this.handleBuildDirCreationRequest();
            }
            return false;
        });
    }

    async handleBuildDirCreationRequest(): Promise<boolean> {
        logger.debug('Create build folder');

        const response = await this.checkProjectConfiguration(ConfigMode.CREATE_BUILD_DIR);
        logger.debug(`Received response: ${response}`);

        switch (response.getType()) {
            case ProjectConfigStatus.IS_OK: {
                const message = 'Build folder is created successfully';
                logger.info(message);
                messages.showInfoMessage(`${message}. Now continuing project configure.`);
                return this.configure(ConfigMode.CHECK);
            }
            case ProjectConfigStatus.BUILD_DIR_CREATION_FAILED: {
                return this.createBuildDirFailed(response.getMessage());
            }
            default: {
                this.handleUnexpectedResponse();
                return false;
            }
        }
    }

    async handleJsonFilesNotFound(notFoundResponseType: ProjectConfigStatus): Promise<boolean> {
        logger.info("Json files not found.");

        const missingFileName = (notFoundResponseType === ProjectConfigStatus.COMPILE_COMMANDS_JSON_NOT_FOUND ? 'compile_commands.json' : 'link_commands.json');
        const configureButton = 'Configure';
        return await vs.window.showWarningMessage(`Project is not configured properly: '${missingFileName}' is missing in the build folder. ` +
            `Please, follow the [guide](${ProjectConfig.guideUri}) to fix it, or UTBot can try to configure it automatically.`, ...[configureButton]).then(async (selection) => {
                if (selection === configureButton) {
                    return await utbotUI.progresses().withProgress(async (progressKey) => {
                        utbotUI.progresses().report(progressKey, "Configuring project, please wait...");
                        const response = await this.checkProjectConfiguration(ConfigMode.GENERATE_JSON_FILES);
                        logger.debug(`Received response: ${response}`);
                        switch (response.getType()) {
                            case ProjectConfigStatus.IS_OK: {
                                const message = 'Successfully configured project';
                                logger.info(message);
                                messages.showInfoMessage(`${message}. Now project is ready for the test generation 👍`);
                                return true;
                            }
                            case ProjectConfigStatus.RUN_JSON_GENERATION_FAILED: {
                                const message = `UTBot tried to configure project, but failed with following message: "${response.getMessage()}"`;
                                logger.warn(message);
                                messages.showWarningMessage(`${message}.
                                                            Please, follow the [guide](${ProjectConfig.guideUri}) to configure project.`);
                                return false;
                            }
                            default: {
                                this.handleUnexpectedResponse();
                                return false;
                            }
                        }
                    });
                }
                return false;
            });
    }
}
