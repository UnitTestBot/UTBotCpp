/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

import * as vs from 'vscode';
import { GTestInfo } from './cache/testsCache';
import { Client } from './client/client';
import { ClientEventsEmitter } from './client/clientEventsEmitter';
import { TestLensProvider } from './codelens/codelensProvider';
import { Commands } from './config/commands';
import * as cfg from './config/config';
import * as messages from './config/notificationMessages';
import { Prefs } from './config/prefs';
import { ProjectConfig } from './config/projectConfig';
import { ProjectConfigEventsEmitter } from './config/projectConfigEventsEmitter';
import { UTBotExplorer } from './explorer/utbotExplorer';
import { UtbotExplorerEventsEmitter } from './explorer/utbotExplorerEventsEmitter';
import { UTBotProjectTarget, UTBotProjectTargetsList as UTBotProjectTargetsInfo } from './explorer/UTBotProjectTarget';
import { UTBotTargetsStorage } from './explorer/utbotTargetsStorage';
import * as gen from './generators/gen';
import * as stubsGen from './generators/stubsGen';
import { utbotUI } from './interface/utbotUI';
import { ExtensionLogger } from './logger';
import { TestsRunner } from './runner/testsRunner';
import * as pathUtils from './utils/pathUtils';
import { executeCommand, getExtensionCommands, registerTextEditorCommand, registerCommand } from './utils/utils';
import { UtbotWizardPanel } from './wizard/wizard';
import { WizardEventsEmitter } from './wizard/wizardEventsEmitter';
import {ConfigMode} from "./proto-ts/testgen_pb";
const { logger } = ExtensionLogger;

export async function activate(context: vs.ExtensionContext): Promise<any> {
	const api = {
		async setProjectPath(projectPath: string): Promise<void> {
			await Prefs.setRemotePath(projectPath);
		}
	};
	const clientEvents = ClientEventsEmitter.getClientEventsEmitter();
	const wizardEvents = WizardEventsEmitter.getWizardEventsEmitter();
	const explorerEvents = UtbotExplorerEventsEmitter.getUbotExplorerEventsEmitter();
	const projectConfigEvents = ProjectConfigEventsEmitter.getProjectConfigEventsEmitter();

	utbotUI.indicators().showAll();
	utbotUI.channels().outputClientLogChannel.show(true);

	const client = new Client(context);

	/**
	 * Initialize test runner
	 */
	const testsRunner = new TestsRunner(context);

	/**
	* Initializes custom container & view.
	*/
	let ignoredUtbotExplorer;
	try {
		ignoredUtbotExplorer = new UTBotExplorer(context);
	} catch (e) {
		logger.error(e);
	}

	setEventsHandlers();

	registerCommands();
	registerProviders();

	// Makes button's icon consistent with property in settings
	setVerboseTestsStatusBarItemTitle();
	await initClient();

	if (Prefs.isNotInitialized(context) && pathUtils.getRootPath()) {
		await explorerEvents.onDidRequestSetDefaultSourceFoldersEventsEmmiter.fire();
		await executeCommand(Commands.InitWizardWebview);
	}

	async function initClient(): Promise<void> {
		await client.setUpClient();
	}

	function setEventsHandlers(): void {
		vs.workspace.onDidChangeConfiguration((_event) => {
			setVerboseTestsStatusBarItemTitle();
		});

		vs.workspace.onDidChangeConfiguration(async (event) => {
			if (event.affectsConfiguration(Prefs.HOST_PREF) ||
				event.affectsConfiguration(Prefs.PORT_PREF)) {
				await initClient();
			}
		});

		vs.workspace.onDidChangeConfiguration(async (event) => {
			if (event.affectsConfiguration(Prefs.BUILD_DIR_PREF)) {
				await configureProject();
			}
		});

		clientEvents.onDidConnectFirstTimeEventEmitter.on(async () => {
			if (Prefs.isInitialized(context)) {
				const success = await configureProject();
				if (success) {
					await stubsGen.stubProject(client);
				}
			}
		});

		wizardEvents.onDidCloseWizardEventEmitter.on(async () => {
			void Prefs.markAsInitialized(context);
		});

		explorerEvents.onDidRequestTargetsEmitter.on(async () => {
			const projectTargets = await utbotUI.progresses().withProgress(async (progressKey, token) => {
				utbotUI.progresses().report(progressKey, "Request project targets...");
				return client.requestProjectTargets(progressKey, token);
			});
			const utbotProjectTargets = projectTargets.getTargetsList().map(target => UTBotProjectTarget.fromProjectTarget(target));
			const projectTargetsPaths = projectTargets.getTargetsList().map(target => target.getPath());
            const serverPriorityTarget = projectTargets.getPrioritytarget();
            const priorityTargetIsValid = (UTBotTargetsStorage.instance.primaryTargetPath
                && projectTargetsPaths.includes(UTBotTargetsStorage.instance.primaryTargetPath));
            let utbotProjectTargetsInfo: UTBotProjectTargetsInfo;
            if (priorityTargetIsValid || serverPriorityTarget === undefined) {
				const clientPriorityTarget =
				utbotProjectTargets.filter(target => target.path === UTBotTargetsStorage.instance.primaryTargetPath)[0];
                utbotProjectTargetsInfo = new UTBotProjectTargetsInfo(utbotProjectTargets, clientPriorityTarget);
            } else {
                const utbotPriorityTarget = UTBotProjectTarget.fromProjectTarget(serverPriorityTarget);
                utbotProjectTargetsInfo = new UTBotProjectTargetsInfo(utbotProjectTargets, utbotPriorityTarget);
            }
			await explorerEvents.onDidAddTargetsEventEmitter.fire(utbotProjectTargetsInfo);
		});

		projectConfigEvents.onDidImportProject.on(() => {
			return explorerEvents.onDidRequestTargetsEmitter.fire();
		});

	}

	function registerCommands(): void {

		/**
		 * Register commands for UTBot Wizard
		 */
		context.subscriptions.push(
			registerCommand(Commands.InitWizardWebview, () => {
				UtbotWizardPanel.createOrShow(context);
			})
		);

		/**
		 * Register commands for UTBot Settings
		 */
		context.subscriptions.push(
			registerCommand(Commands.SelectLoggingLevel, async () => {
				await client.selectLoggingLevel();
			}),
			registerCommand(Commands.UpdateVerboseTestFlag, async () => {
				await updateVerboseTestMode();
			})
		);

		/**
		 * Register commands for Project Configuration
		 */
		context.subscriptions.push(
			registerCommand(Commands.ConfigureProject, async (): Promise<void> => {
				await configureProject();
			})
		);

		/**
		 * Register commands for Project Configuration
		 */
		context.subscriptions.push(
			registerCommand(Commands.ReConfigureProject, async (): Promise<void> => {
				await reConfigureProject();
			})
		);


		/**
		 * Register commands for Tests Generation
		 */
		context.subscriptions.push(
			registerTextEditorCommand(Commands.GenerateTestsForIsolatedFile, genSnippetCallback),
			registerCommand(Commands.GenerateTestsForProject, genProjectCallback),
			registerCommand(Commands.GenerateTestsForFolder, genFolderCallback),
			registerTextEditorCommand(Commands.GenerateTestsForFile, genFileCallback),
			registerTextEditorCommand(Commands.GenerateTestsForFunction, genFunctionCallback),
			registerTextEditorCommand(Commands.GenerateTestsForClass, genClassCallback),
			registerTextEditorCommand(Commands.GenerateProjectLineTests, genLineCallback),
			registerTextEditorCommand(Commands.GenerateAssertionFailTests, genAssertionCallback),
			registerTextEditorCommand(Commands.GeneratePredicateTests, genPredicateCallback)
		);

		/**
		 * Register helping commands for tests generation that are used for menu items with short names.
		 * We use separate commands due to naming differencies: vscode api doesn't provide a possibility to set
		 * menu item name and only uses commands name.
		 */
		context.subscriptions.push(
			registerTextEditorCommand(Commands.GenerateTestsForFileMenu, genFileCallback),
			registerTextEditorCommand(Commands.GenerateTestsForFunctionMenu, genFunctionCallback),
			registerTextEditorCommand(Commands.GenerateTestsForClassMenu, genClassCallback),
			registerTextEditorCommand(Commands.GenerateProjectLineTestsMenu, genLineCallback),
			registerTextEditorCommand(Commands.GenerateAssertionFailTestsMenu, genAssertionCallback),
			registerTextEditorCommand(Commands.GeneratePredicateTestsMenu, genPredicateCallback)
		);

		/**
		 * Register commands for Tests Runner
		 */
		context.subscriptions.push(
			registerCommand(Commands.RunAllTestsAndShowCoverage, async () => {
				await testsRunner.run(client, undefined);
			}),
			registerCommand(Commands.RunSpecificTestAndShowCoverage, async (testInfo: GTestInfo) => {
				await testsRunner.run(client, testInfo);
			})
		);

		/**
		 * Register commands for Stubs Generation
		 */
		context.subscriptions.push(
			registerTextEditorCommand(Commands.GenerateStubsForProject, async () => {
				await stubsGen.stubProject(client);
			}),
			registerCommand(Commands.HideCoverageGutters, async () => {
				testsRunner.hideTestResultsAndCoverage();
			}),
			registerCommand(Commands.PrintModulesContent, async () => {
				await client.requestPrintModulesContent();
			})
		);

		/**
		 * Register Meta Commands
		 */
		context.subscriptions.push(
			registerCommand(Commands.ShowAllCommands, async () => {
				await showAllCommands();
			}),
			registerCommand(Commands.OpenBuildDirectory, async () => {
				await executeCommand('workbench.action.openSettings', 'unittestbot.paths.buildDirectory');
			})
		);
	}

	function registerProviders(): void {
		const tlp = new TestLensProvider();
		vs.languages.registerCodeLensProvider(
			[
				cfg.docSelectorC,
				cfg.docSelectorCpp
			],
			tlp
		);
	}

	function setVerboseTestsStatusBarItemTitle(): void {
		try {
			const verboseStatusItem = utbotUI.indicators().verboseTestsStatusBarItem;
			if (Prefs.isVerboseTestModeSet()) {
				verboseStatusItem.text = utbotUI.titles.FIVE_RULES_ON;
			} else {
				verboseStatusItem.text = utbotUI.titles.FIVE_RULES_OFF;
			}
		} catch {
			messages.showErrorMessage(`Can't get property's value (verbose tests)`);
			logger.error(`Can't get property's value (verbose tests)`);
		}
	}

	async function updateVerboseTestMode(): Promise<void> {
		try {
			await Prefs.setVerboseTestMode(!Prefs.isVerboseTestModeSet());
		} catch {
			messages.showErrorMessage(`Can't set/get property's value (verbose tests)`);
			logger.error(`Can't set/get property's value (verbose tests)`);
		}
	}


	async function showAllCommands(): Promise<void> {
		const extCommands = getExtensionCommands();
		await vs.window.showQuickPick(
			extCommands.filter(cmd =>
				!Commands.MenuCommands.includes(cmd.command) &&
				!Commands.UTbotFoldersCommands.includes(cmd.command) &&
				!Commands.UTbotTargetsCommands.includes(cmd.command)
			).map(cmd => cmd.title)
		)
			.then(async cmdTitle => {
				const selectedCmd = extCommands.find(x => x.title === cmdTitle);
				if (selectedCmd) {
					await executeCommand(selectedCmd.command);
				}
			});
	}

	async function configureProject(): Promise<boolean> {
		const projectConfig = new ProjectConfig(client);
		return projectConfig.configure(ConfigMode.CHECK);
	}

	async function reConfigureProject(): Promise<boolean> {
		const projectConfig = new ProjectConfig(client);
		return projectConfig.configure(ConfigMode.ALL);
	}

	async function genSnippetCallback(editor?: vs.TextEditor): Promise<void> {
		const activeFilePath = editor?.document.uri.fsPath;
		await gen.isolatedRun(client, testsRunner, activeFilePath);
	}

	async function genProjectCallback(): Promise<void> {
		await gen.run(gen.GenTestsMode.Project, client, testsRunner);
	}

	async function genFolderCallback(): Promise<void> {
		await gen.run(gen.GenTestsMode.Folder, client, testsRunner);
	}

	async function genFileCallback(editor: vs.TextEditor): Promise<void> {
		const activeFilePath = editor?.document.uri.fsPath;
		await gen.run(gen.GenTestsMode.File, client, testsRunner, editor, activeFilePath);
	}

	async function genFunctionCallback(editor: vs.TextEditor): Promise<void> {
		const activeFilePath = editor?.document.uri.fsPath;
		await gen.run(gen.GenTestsMode.Function, client, testsRunner, editor, activeFilePath);
	}

	async function genClassCallback(editor: vs.TextEditor): Promise<void> {
		const activeFilePath = editor?.document.uri.fsPath;
		await gen.run(gen.GenTestsMode.Class, client, testsRunner, editor, activeFilePath);
	}

	async function genLineCallback(editor: vs.TextEditor): Promise<void> {
		const activeFilePath = editor?.document.uri.fsPath;
		await gen.run(gen.GenTestsMode.Line, client, testsRunner, editor, activeFilePath);
	}

	async function genAssertionCallback(editor: vs.TextEditor): Promise<void> {
		const activeFilePath = editor?.document.uri.fsPath;
		await gen.run(gen.GenTestsMode.Assertion, client, testsRunner, editor, activeFilePath);
	}

	async function genPredicateCallback(editor: vs.TextEditor): Promise<void> {
		const activeFilePath = editor?.document.uri.fsPath;
		await gen.run(gen.GenTestsMode.Predicate, client, testsRunner, editor, activeFilePath);
	}
	return api;
}
