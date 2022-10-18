import { ExtensionLogger } from '../logger';
import { ITestsGenServiceClient } from '../proto-ts/testgen_grpc_pb';
import { VersionInfo } from '../proto-ts/testgen_pb';
const { logger } = ExtensionLogger;

export class GrpcServicePing {

    constructor(
        public readonly clientVersion: string,
        public readonly service: ITestsGenServiceClient) {}

    /**
     * Returns true if server responds in a given time (milliseconds).
     */ 
    public async ping(timeout: number): Promise<string|null> {
        let responded = null;
        await Promise.race([
            this.pingInner(),
            this.timeoutPromise(timeout)
        ])
        .then(serverVer => {
            logger.error(`Server version: ${serverVer}`);
            responded = serverVer;
        })
        .catch(err => {
            logger.error(`Server failed to respond: ${err}`);
        });
        return responded;
    }

    private async pingInner(): Promise<string> {
        const versionInfo = new VersionInfo();
        versionInfo.setVersion(this.clientVersion);
        return new Promise<string>((resolve, reject) => {
           this.service.handshake(versionInfo, (err, _response) => {
                if (err) {
                    reject(`Error: ${err}`);
                } else {
                    resolve(_response.getVersion());
                }
            });
        });
    }


    private async timeoutPromise(timeout: number): Promise<void> {
        return new Promise((_ignoredResolve, reject) => {
            const wait = setTimeout(() => {
                clearTimeout(wait);
                reject("Timeout");
              }, timeout);
        });
        
    }


}
