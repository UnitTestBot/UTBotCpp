import * as vs from 'vscode';


export const portNotFoundError = "Cannot launch server: port not found. Please, try again.";
export const fileNotOpenedError = "There are no opened text files.";
export const successfullyCanceledInfo = "The process has been cancelled.";
export const successfullyConnected = "Successfully connected to UTBot server.";
export const testGenerationFailedInfo = "UTBot failed to generate test(s).";
export const coverageGenerationFailedInfo = "UTBot failed to generate coverage.";
export const serverIsDeadError = "UTBot server doesn't respond. Check the connection, please.";
export const grpcConnectionLostError = "No connection established";
export const targetNotUsed = "There is no used target. Use any in UTBot Targets window, please.";


export function showErrorMessage(err: any): void {
    let errorMessage = getErrorMessage(err);
    if (errorMessage === grpcConnectionLostError) {
        errorMessage = serverIsDeadError;
    }
    // eslint-disable-next-line @typescript-eslint/no-floating-promises
    vs.window.showErrorMessage(errorMessage);
}

export function showWarningMessage(message: string): void {
    // eslint-disable-next-line @typescript-eslint/no-floating-promises
    vs.window.showWarningMessage(message);
}

export function showInfoMessage(message: string): void {
    // eslint-disable-next-line @typescript-eslint/no-floating-promises
    vs.window.showInformationMessage(message);
}

function getErrorMessage(err: any): any {
    if (err.details !== undefined) {
        return err.details;
    } else if (err.message !== undefined) {
        return err.message;
    }
    return err;
}
