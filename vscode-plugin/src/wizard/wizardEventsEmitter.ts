import { UTBotEventEmitter } from "../emitter/UTBotEventEmitter";

export class WizardEventsEmitter {

    private constructor() {}

    private static instance: WizardEventsEmitter;

    public static getWizardEventsEmitter(): WizardEventsEmitter {
        if (!this.instance) {
            this.instance = new WizardEventsEmitter();
        }

        return this.instance;
    }

    readonly onDidCloseWizardEventEmitter: UTBotEventEmitter<void> = 
        new UTBotEventEmitter();
}
