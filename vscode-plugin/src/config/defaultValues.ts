import * as path from 'path';
import * as vs from 'vscode';
import * as vsUtils from '../utils/vscodeUtils';
import { Prefs } from './prefs';
import * as pathUtils from '../utils/pathUtils';
import { isIP } from 'net';
import {isWin32} from "../utils/utils";

export class DefaultConfigValues {
    public static readonly DEFAULT_HOST = "localhost";
    public static readonly DEFAULT_GRPC_PORT = 2121;
    public static readonly DEFAULT_SFTP_PORT = 2020;

    public static readonly POSSIBLE_BUILD_DIR_NAMES = ['out', 'build'];
    public static readonly POSSIBLE_TEST_DIR_NAMES = ['test'];

    public static readonly DEFAULT_CMAKE_OPTIONS = ['-DCMAKE_EXPORT_COMPILE_COMMANDS=ON', '-DCMAKE_EXPORT_LINK_COMMANDS=ON'];

    public static toWSLPathOnWindows(path: string): string {
        if (!isWin32()) {
            return path;
        }
        return path
            .replace(/^(\w):|\\+/g,'/$1')
            .replace(/^\//g,'/mnt/');
    }

    public static hasConfiguredRemotePath(): boolean {
        return Prefs.getRemotePath().length !== 0;
    }

    public static getDefaultHost(): string {
        let host = Prefs.getGlobalHost();
        if (!DefaultConfigValues.hasConfiguredRemotePath()) {
            const sftHost = vsUtils.getFromSftpConfig("host");
            if (sftHost && isIP(sftHost)) {
                host = sftHost;
            }
        }
        return host;
    }

    public static getDefaultGRPCPort(): number {
        return parseInt(Prefs.getGlobalGRPCPort());
    }

    public static getDefaultSFTPPort(): number {
        const sftpPort = vsUtils.getFromSftpConfig("port");
        if (sftpPort !== undefined) {
            return parseInt(sftpPort);
        }
        return DefaultConfigValues.DEFAULT_SFTP_PORT;
    }

    public static getDefaultSFTPUsername(): string{
        const username = vsUtils.getFromSftpConfig("username");
        if ((username === undefined) || (username.length === 0)) {
            return "utbot";
        }
        return username.toString();
    }

    public static getDefaultSFTPPassword(): string{
        const password = vsUtils.getFromSftpConfig("password");
        if ((password === undefined) || (password.length === 0)) {
            return "utbot";
        }
        return password.toString();
    }

    public static getDefaultRemotePath(): string {
        let remotePath = Prefs.getRemotePath();
        if (remotePath.length === 0) {
            if (DefaultConfigValues.getDefaultHost() === vsUtils.getFromSftpConfig("host")) {
                // if `host` is the same as SFTP host, inherit the remote path from SFTP plugin
                const sftpRemotePath = vsUtils.getFromSftpConfig("remotePath");
                if (sftpRemotePath) {
                    remotePath = sftpRemotePath;
                }
            } else {
                remotePath = DefaultConfigValues.toWSLPathOnWindows(vsUtils.getProjectDirByOpenedFile().fsPath);
            }
        }
        return remotePath;
    }

    public static async getDefaultBuildDirectoryPath(): Promise<string> {
        // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
        const rootPath = pathUtils.getRootPath();
        let buildDirName = "build";
        if (!rootPath) {
            return buildDirName;
        }
        await vs.workspace.fs.readDirectory(vs.Uri.file(rootPath))
            .then(resultArray => {
                resultArray.forEach(([name, type]) => {
                    // add only non-hidden directories and not a build directory by default
                    if (type === vs.FileType.Directory
                        && DefaultConfigValues.looksLikeBuildDirectory(name)) {
                        buildDirName = name;
                    }
                });
            });
        return buildDirName;
    }

    public static looksLikeBuildDirectory(dirPath: string): boolean {
        return this.looksLike(dirPath, DefaultConfigValues.POSSIBLE_BUILD_DIR_NAMES);
    }

    public static looksLikeTestDirectory(dirPath: string): boolean {
        return this.looksLike(dirPath, this.POSSIBLE_TEST_DIR_NAMES);
    }

    private static looksLike(dirPath: string, patterns: string[]): boolean {
        const name = path.basename(dirPath).toLowerCase();
        let looksLikeOneOfPatterns = false;
        patterns.forEach(possibleName => {
            if (name.includes(possibleName)) {
                looksLikeOneOfPatterns = true;
            }
        });

        return looksLikeOneOfPatterns;
    }
}
