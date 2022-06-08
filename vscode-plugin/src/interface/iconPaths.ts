import { ExtensionContext } from 'vscode';

export type IconPath = string | { dark: string; light: string };

export class IconPaths {

	passed: IconPath;
	failed: IconPath;
	errored: IconPath;
	fullCoverage: IconPath;
	oldCoverage: IconPath;
	partialCoverage: IconPath;
	noCoverage: IconPath;
	folderDefault: IconPath;
	folderDefaultOpen: IconPath;
	folderUtbot: IconPath;
	folderUtbotOpen: IconPath;
	file: IconPath;
	targetUnused: IconPath;
	targetUsed: IconPath;


	constructor(context: ExtensionContext) {
		this.passed = context.asAbsolutePath('icons/passed.svg');
		this.failed = context.asAbsolutePath('icons/failed.svg');
		this.errored = context.asAbsolutePath('icons/errored.svg');
		this.fullCoverage = {
			dark: context.asAbsolutePath("./app_images/gutter-icon-dark.svg"), 
			light: context.asAbsolutePath("./app_images/gutter-icon-light.svg")
		};
		this.oldCoverage = {
			dark: context.asAbsolutePath("./app_images/old-gutter-icon-dark.svg"),
			light: context.asAbsolutePath("./app_images/old-gutter-icon-light.svg")
		};
		this.partialCoverage = {
			dark: context.asAbsolutePath("./app_images/partial-gutter-icon-dark.svg"),
			light: context.asAbsolutePath("./app_images/partial-gutter-icon-light.svg")
		};
		this.noCoverage = {
			dark: context.asAbsolutePath("./app_images/no-gutter-icon-dark.svg"),
			light: context.asAbsolutePath("./app_images/no-gutter-icon-light.svg")
		};
		this.folderDefault = context.asAbsolutePath('icons/folder-defaut.svg');
		this.folderDefaultOpen = context.asAbsolutePath('icons/folder-default-open.svg');
		this.folderUtbot = context.asAbsolutePath('icons/folder-utbot.svg');
		this.folderUtbotOpen = context.asAbsolutePath('icons/folder-utbot-open.svg');
		this.file = context.asAbsolutePath('icons/file.svg');
		this.targetUnused = {
			dark: context.asAbsolutePath('icons/target-unused-dark.svg'),
			light: context.asAbsolutePath('icons/target-unused-light.svg')
		};
		this.targetUsed = {
			dark: context.asAbsolutePath('icons/target-used-dark.svg'),
			light: context.asAbsolutePath('icons/target-used-light.svg')
		};
	}
}
