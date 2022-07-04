import * as Emittery from "emittery";

export class UTBotEventEmitter<T>  {
    private emittery: Emittery<Record<string, T>> = new Emittery();
    private KEY = '';

    public fire(data: T): Promise<void> {
        return this.emittery.emit(this.KEY, data);
    }

    public on(listener: (eventData: T) => void | Promise<void>): void {
        this.emittery.on(this.KEY, listener);
    }
}
