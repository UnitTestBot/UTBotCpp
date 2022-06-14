/**
 * A storage for some data of type In, that can be loaded via loadData method and stored as type Out.
 * Stored data can be retrived as Out via getLoadedData().
 * Simply, it's a storage that converts data.
 */
export interface DataLoader<In, Out> {

    /**
     * Transforms data data and add it to its state.
     * @param data
     */
    loadData(data: Array<In> | In | undefined): Promise<void>;

    /**
     * Returns stored data.
     */
    getLoadedData(): Out;

    /**
     * Clears all stored data.
     */
    clearData(): void;
}
