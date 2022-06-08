import * as log4js from "log4js";
import * as vs from "vscode";
import { Logger } from "../logger";

export class TestLogger {
    private static readonly DEFAULT_LOG_LEVEL = 'DEBUG';
    private static readonly outputClientLogChannel = vs.window.createOutputChannel('IT channel');

    private static readonly testLogger = Logger.getLog4JsLogger(TestLogger.DEFAULT_LOG_LEVEL,
        TestLogger.outputClientLogChannel);

    public static getLogger(): log4js.Logger {
        return TestLogger.testLogger;
    }
}