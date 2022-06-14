import { ExtensionLogger } from '../logger';
import { ITestsGenServiceClient } from '../proto-ts/testgen_grpc_pb';
import { DummyRequest } from '../proto-ts/testgen_pb';
const { logger } = ExtensionLogger;

export class GrpcServicePing {

    constructor(public readonly service: ITestsGenServiceClient) {}

    /**
     * Returns true if server responds in a given time (milliseconds).
     */ 
    public async ping(timeout: number): Promise<boolean> {
        let responded = true;
        await Promise.race([
            this.pingInner(),
            this.timeoutPromise(timeout)
        ])
        .then(_value => {
            responded = true;
        })
        .catch(err => {
            logger.error(`Server failed to respond: ${err}`);
            responded = false;
        });
        return responded;
    }

    private async pingInner(): Promise<void> {
        const dummyRequest = new DummyRequest();
        return new Promise<void>((resolve, reject) => {
           this.service.handshake(dummyRequest, (err, _response) => {
                if (err) {
                    reject(`Error: ${err}`);
                } else {
                    resolve();
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
