import * as log4js from "log4js";
/**
 * Makes it possible to build code source map.
 * Creates mapping between TypeScript code and
 * transpiled JavaScript code.
 */
import 'source-map-support/register';
import * as vs from 'vscode';
import { utbotUI } from "./interface/utbotUI";


export type LogLevel = 'FATAL' | 'ERROR' | 'DEBUG' | 'TRACE' | 'INFO' | 'WARN';

export class ExtensionLogger {
    private static _logger: log4js.Logger | undefined;

    public static get logger(): log4js.Logger {
        if (ExtensionLogger._logger === undefined) {
            ExtensionLogger._logger = Logger.getLog4JsLogger(
                'INFO', 
                utbotUI.channels().outputClientLogChannel
            );
        }
        // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
        return ExtensionLogger._logger!;
    }

    public static setupLogger(configLogLevel: LogLevel, outputLogChannel: vs.OutputChannel): void {
        ExtensionLogger._logger = Logger.getLog4JsLogger(configLogLevel, outputLogChannel);
    }
}

export class Logger {
    private static patternString = '%d{yyyy-MM-dd hh:mm:ss} | %20.25f{1}:%3.100l |%1.1p| %m%n';
    private static infoPatternString = '%d{hh:mm:ss} | %m%n';

    constructor(){}

    public static getLog4JsLogger(configLogLevel: LogLevel, outputLogChannel: vs.OutputChannel): log4js.Logger {
        return this.configureLog4js(configLogLevel, outputLogChannel);
    }

    /**
     * Configurates Log4js logger.
     */
    private static configureLog4js(configLogLevel: LogLevel, outputLogChannel: vs.OutputChannel): log4js.Logger {
        /** 
         * Creating custom appender, that writes logs to 
         * VS Code output channel 
        */
        const vscodeOutChannelAppender = {
            configure: (config: any, layouts: any): {} => {
                const vscodeAppender = (loggingEvent: log4js.LoggingEvent): void => {
                    let layout = layouts.basicLayout;
                    if (config.layout) {
                        layout = layouts.layout(config.layout.type, config.layout);
                    }
                    outputLogChannel.append(`${layout(loggingEvent, config.timezoneOffset)}`);
                };

                vscodeAppender.shutdown = (done: any): void => {
                    outputLogChannel.dispose();
                    setTimeout(done, 10);
                };
                return vscodeAppender;
            }
        };

        log4js.configure({
            appenders: {
                vscodeOutChannel: {
                    type: vscodeOutChannelAppender,
                    layout: {
                        type: 'pattern',
                        pattern: this.patternString
                    }
                },
                console: {
                    type: 'console',
                    layout: {
                        type: 'pattern', 
                        pattern: this.patternString
                    }
                },
                vscodeOutChannelForClientOnlyAppender : {
                    type: vscodeOutChannelAppender,
                    layout: {
                        type: 'pattern',
                        pattern: this.infoPatternString
                    }
                },
                vscodeOutChannelForInfo: {
                    type: 'logLevelFilter',
                    level: 'info',
                    appender: 'vscodeOutChannelForClientOnlyAppender'
                },
                vscodeOutChannelForDebug: {
                    type: 'logLevelFilter',
                    level: 'trace',
                    maxLevel: 'debug',
                    appender: 'vscodeOutChannel'
                }

            },
            categories: {
                default: { appenders: ['vscodeOutChannelForInfo', 'vscodeOutChannelForDebug', 'console'], level: configLogLevel, enableCallStack: true },
            }
        });

        return log4js.getLogger("Client");
    }
   
    public static configToLog4jsLogLevel(configLogLevel: LogLevel): string {
        return configLogLevel.toLowerCase();
    }
}
