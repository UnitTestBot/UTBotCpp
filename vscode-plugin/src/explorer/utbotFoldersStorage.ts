import * as path from 'path';
import { Prefs } from '../config/prefs';
import { FSUtils } from '../utils/fsUtils';
import * as vsUtils from '../utils/vscodeUtils';

export type Path = string;

export class UTBotFoldersStorage {
    private constructor() { }

    private static _instance: UTBotFoldersStorage | undefined = undefined;
    private folders: Set<Path> = new Set();

    public static get instance(): UTBotFoldersStorage {
        if (this._instance === undefined) {
            this._instance = new UTBotFoldersStorage();
            this._instance.setFoldersFromConfiguration();
        }

        return this._instance;
    }

    public setFoldersFromConfiguration(): void {
        this.clear();
        const configPaths = Prefs.getSourcePathsRootsFromConfig();
        configPaths.forEach(configPath => {
            this.addFolder(configPath);
        });
    }

    public async updateConfigurationFromCurrentState(): Promise<void> {
        const folders = Array.from(this.folders);
        const relativePaths = folders.map(folder => {
            let relativePath = path.relative(vsUtils.getProjectDirByOpenedFile().fsPath, folder);
            relativePath = relativePath.split(path.sep).join(path.posix.sep);
            if (relativePath.length === 0) { // is root directory
                relativePath = '.';
            }
            return relativePath;
        });
        await Prefs.setAsset<string>(
            Prefs.SOURCE_DIRS_PREF,
            relativePaths.filter(relPath => relPath.length > 0).join(","),
            false
        );
    }

    public getFolders(): Set<Path> {
        return this.folders;
    }

    private clear(): void {
        this.folders = new Set();
    }

    public addFolder(folder: Path): void {
        this.folders.add(folder);
    }

    public addFolderRecursive(folder: Path): void {
        // Just a small optimization
        if (!this.containsFolder(folder)) {
            this.addFolder(folder);
            const subDirs = FSUtils.getDirectoriesRecursive(folder);
            this.addFolders(subDirs);
        }
    }

    public containsFolder(folder: Path): boolean {
        let containsFlag: boolean = false;
        this.folders.forEach(folderPath => {
            if (folderPath === folder) {
                containsFlag = true;
            }
        });

        return containsFlag;
    }


    public addFolders(folderUris: Path[]): void {
        for (const folderUri of folderUris) {
            this.addFolder(folderUri);
        }
    }

    public addFoldersRecursive(folders: Path[]): void {
        for (const folderUri of folders) {
            this.addFolderRecursive(folderUri);
        }
    }

    public removeFolder(folder: Path): void {
        this.folders.forEach((usedFolder) => {
            if (usedFolder === folder) {
                this.folders.delete(usedFolder);
            }
        });
    }

    public removeFolderRecursive(folder: Path): void {
        this.folders.forEach((usedFolder) => {
            if (usedFolder.startsWith(folder)) {
                this.folders.delete(usedFolder);
            }
        });
    }

    public removeFoldersRecursive(folders: Path[]): void {
        for (const folder of folders) {
            this.removeFolderRecursive(folder);
        }
    }

    public removeFolders(folders: Path[]): void {
        for (const folder of folders) {
            this.removeFolder(folder);
        }
    }
}
