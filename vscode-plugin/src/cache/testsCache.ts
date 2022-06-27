export class GTestInfo {
    constructor(public readonly filePath: string,
                public readonly testSuite: string,  
                public readonly testName: string,
                public lineNumber: number) {}
}

export type filename = string;

export class TestsCache {
    private cache: Map<filename, GTestInfo[]>;
    private static instance: TestsCache;


    private constructor() {
        this.cache = new Map();
    }
   
    public static getCache(): TestsCache {
        if (this.instance === undefined) {
            this.instance = new TestsCache();
        }

        return this.instance;
    }

    public put(testInfo: GTestInfo): void {
        const fileName = testInfo.filePath;
        const tests = this.cache.get(fileName);

        if (tests === undefined) {
            const newTests = [testInfo];
            this.cache.set(fileName, newTests);
            return;
        }

        // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
        const index = tests!.findIndex(test => 
            test.filePath === testInfo.filePath 
            && test.testName === testInfo.testName);

        if (index === -1) {
            // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
            this.cache.get(fileName)!.push(testInfo);
        } else {
            // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
            this.cache.get(fileName)![index] = testInfo;
        }
    }

    public get(fileName: filename, testName: string): GTestInfo | undefined {
        const tests = this.cache.get(fileName);
        if (tests === undefined) {
            return undefined;
        }
        const index = tests?.findIndex(test => 
            test.filePath === fileName 
            && test.testName === testName);
        if (index === -1) {
            return undefined;
        } else {
            return tests[index];
        }
    }

    public clear(): void {
        this.cache = new Map();
    }
}
