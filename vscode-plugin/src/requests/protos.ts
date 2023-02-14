import { Prefs } from "../config/prefs";
import {
    ClassRequest,
    FileRequest,
    FolderRequest,
    FunctionRequest,
    LineRequest,
    PredicateRequest,
    ProjectContext,
    ProjectRequest
} from "../proto-ts/testgen_pb";
import { PredicateInfo, SourceInfo, ValidationType } from "../proto-ts/util_pb";
import { RequestTestsParams } from "./params";
import * as vsUtils from "../utils/vscodeUtils";


export class Protos {
    public static projectRequestByParams(params: RequestTestsParams): ProjectRequest {
        return this.projectRequest(
            params.projectPath,
            params.buildDirRelativePath,
            params.projectName,
            params.sourcePaths,
            params.synchronizeCode,
            params.targetPath);
    }

    public static projectRequest(
        projectPath: string,
        buildDirRelativePath: string,
        projectName: string,
        srcPathsList: string[],
        synchronizeCode: boolean,
        targetPath: string): ProjectRequest {
        const projectInfo = new ProjectRequest();

        const projectContext = new ProjectContext();
        projectContext.setProjectname(projectName);
        projectContext.setProjectpath(projectPath);
        projectContext.setTestdirpath(Prefs.getTestsDirPath());
        projectContext.setBuilddirrelativepath(buildDirRelativePath);
        projectContext.setClientprojectpath(vsUtils.getProjectDirByOpenedFile().fsPath)
        projectInfo.setProjectcontext(projectContext);
        projectInfo.setSettingscontext(Prefs.getSettingsContext());
        projectInfo.setSourcepathsList(srcPathsList);
        projectInfo.setSynchronizecode(synchronizeCode);
        projectInfo.setTargetpath(targetPath);
        return projectInfo;
    }

    public static sourceInfo(lineInfo: [string, number]): SourceInfo {
        const testSourceInfo = new SourceInfo();
        testSourceInfo.setFilepath(lineInfo[0]);
        testSourceInfo.setLine(lineInfo[1]);
        return testSourceInfo;
    }

    public static testsGenFolderRequest(projectInfo: ProjectRequest,
        folderPath: string): FolderRequest {
        const rpcFolderRequest = new FolderRequest();
        rpcFolderRequest.setProjectrequest(projectInfo);
        rpcFolderRequest.setFolderpath(folderPath);
        return rpcFolderRequest;
    }

    public static testsGenFileRequest(projectInfo: ProjectRequest,
        filePath: string): FileRequest {
        const rpcFileRequest = new FileRequest();
        rpcFileRequest.setProjectrequest(projectInfo);
        rpcFileRequest.setFilepath(filePath);
        return rpcFileRequest;
    }

    public static testsGenFunctionRequest(projectInfo: ProjectRequest,
        testSourceInfo: SourceInfo): FunctionRequest {
        const rpcFunctionRequest = new FunctionRequest();
        const lineRequest = new LineRequest();
        lineRequest.setProjectrequest(projectInfo);
        lineRequest.setSourceinfo(testSourceInfo);
        rpcFunctionRequest.setLinerequest(lineRequest);
        return rpcFunctionRequest;
    }

    public static testsGenClassRequest(projectInfo: ProjectRequest,
        testSourceInfo: SourceInfo): ClassRequest {
        const rpcClassRequest = new ClassRequest();
        const lineRequest = new LineRequest();
        lineRequest.setProjectrequest(projectInfo);
        lineRequest.setSourceinfo(testSourceInfo);
        rpcClassRequest.setLinerequest(lineRequest);
        return rpcClassRequest;
    }

    public static testsGenPredicateRequest(projectInfo: ProjectRequest,
        testSourceInfo: SourceInfo,
        predicateInfo: [ValidationType, string, string]): PredicateRequest {
        const rpcPredicateRequest = new PredicateRequest();
        const rpcLineRequest = new LineRequest();
        rpcLineRequest.setSourceinfo(testSourceInfo);
        rpcLineRequest.setProjectrequest(projectInfo);
        rpcPredicateRequest.setLinerequest(rpcLineRequest);
        const predicateInfo_ = new PredicateInfo();
        predicateInfo_.setType(predicateInfo[0]);
        predicateInfo_.setPredicate(predicateInfo[1]);
        predicateInfo_.setReturnvalue(predicateInfo[2]);
        rpcPredicateRequest.setPredicateinfo(predicateInfo_);
        return rpcPredicateRequest;
    }

}
