package org.utbot.cpp.clion.plugin.grpc

import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTarget
import testsgen.Testgen
import testsgen.Util
import java.nio.file.InvalidPathException
import java.nio.file.Paths


internal data class ProjectContextParams(
    val projectName: String,
    val projectPath: String,
    val testDirRelativePath: String,
    val buildDirRelativePath: String
) : Params<Testgen.ProjectContext> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ProjectContext {
        val projectNioPath = Paths.get(projectPath) // project path is not set by user, assuming it is valid
        val relativeTestsDirNioPath = try {
            Paths.get(testDirRelativePath)
        } catch (e: InvalidPathException) {
            throw IllegalPathException(testDirRelativePath, UTBot.message("settings.project.testsDir.wrong"))
        }
        return Testgen.ProjectContext.newBuilder()
            .setProjectPath(projectPath)
            .setTestDirPath(
                remoteMapping.convertToRemote(
                    projectNioPath.resolve(relativeTestsDirNioPath).toString(),
                    UTBot.message("settings.project.testsDir.wrong")
                )
            )
            .setBuildDirRelativePath(buildDirRelativePath)
            .setProjectName(projectName)
            .setProjectPath(remoteMapping.convertToRemote(projectPath, UTBot.message("projectPath.wrong.conversion")))
            .build()
    }
}

internal data class SettingsContextParams(
    val generateForStaticFunctions: Boolean,
    val verbose: Boolean,
    val timeoutPerFunction: Int,
    val timeoutPerTest: Int,
    val useDeterministicSearcher: Boolean,
    val useStubs: Boolean
) : Params<Testgen.SettingsContext> {
    override fun build(remoteMapping: RemoteMapping): Testgen.SettingsContext {
        return Testgen.SettingsContext.newBuilder()
            .setVerbose(verbose)
            .setGenerateForStaticFunctions(generateForStaticFunctions)
            .setTimeoutPerFunction(timeoutPerFunction)
            .setTimeoutPerTest(timeoutPerTest)
            .build()
    }
}

internal data class ProjectRequestParams(
    val projectContextParams: ProjectContextParams,
    val settingsContextParams: SettingsContextParams,
    val sourcePaths: List<String>,
    val synchronizeCode: Boolean,
    val targetPath: String
) : Params<Testgen.ProjectRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ProjectRequest {
        return Testgen.ProjectRequest.newBuilder()
            .setProjectContext(projectContextParams.build(remoteMapping))
            .setSettingsContext(settingsContextParams.build(remoteMapping))
            .addAllSourcePaths(sourcePaths.map { sourcePath ->
                remoteMapping.convertToRemote(
                    sourcePath,
                    UTBot.message("settings.project.sourcePaths.wrong.conversion")
                )
            })
            .setSynchronizeCode(synchronizeCode)
            .also { builder ->
                if (!UTBotTarget.isAutoTargetPath(targetPath)) {
                    builder.targetPath = remoteMapping.convertToRemote(
                        targetPath,
                        UTBot.message("settings.project.target.wrong.conversion")
                    )
                }
            }
            .build()
    }
}

internal data class FolderRequestParams(
    val projectRequestParams: ProjectRequestParams,
    val folderPath: String
) : Params<Testgen.FolderRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.FolderRequest {
        return Testgen.FolderRequest.newBuilder()
            .setFolderPath(remoteMapping.convertToRemote(folderPath, UTBot.message("folderPath.wrong.conversion")))
            .setProjectRequest(projectRequestParams.build(remoteMapping))
            .build()
    }
}

internal data class FileRequestParams(
    val projectRequestParams: ProjectRequestParams,
    val filePath: String
) : Params<Testgen.FileRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.FileRequest {
        return Testgen.FileRequest.newBuilder()
            .setProjectRequest(projectRequestParams.build(remoteMapping))
            .setFilePath(remoteMapping.convertToRemote(filePath, UTBot.message("filePath.wrong.conversion")))
            .build()
    }
}

internal data class ProjectConfigRequestParams(
    val projectContextParams: ProjectContextParams,
    val configMode: Testgen.ConfigMode,
    val cmakeOptions: List<String>
) : Params<Testgen.ProjectConfigRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ProjectConfigRequest {
        return Testgen.ProjectConfigRequest.newBuilder()
            .setProjectContext(projectContextParams.build(remoteMapping))
            .setConfigMode(configMode)
            .addAllCmakeOptions(cmakeOptions)
            .build()
    }
}

internal data class ProjectTargetsParams(
    val projectContextParams: ProjectContextParams
) : Params<Testgen.ProjectTargetsRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ProjectTargetsRequest {
        return Testgen.ProjectTargetsRequest.newBuilder()
            .setProjectContext(projectContextParams.build(remoteMapping))
            .build()
    }
}

internal data class TestFilterParams(
    val testFilePath: String,
    val testName: String,
    val testSuite: String
) : Params<Testgen.TestFilter> {
    override fun build(remoteMapping: RemoteMapping): Testgen.TestFilter {
        return Testgen.TestFilter.newBuilder()
            .setTestFilePath(
                remoteMapping.convertToRemote(
                    testFilePath,
                    UTBot.message("testFilePath.wrong.conversion")
                )
            )
            .setTestName(testName)
            .setTestSuite(testSuite)
            .build()
    }
}

internal data class CoverageAndResultsRequestParams(
    val projectContextParams: ProjectContextParams,
    val settingsContextParams: SettingsContextParams,
    val testFilterParams: TestFilterParams? = null,
    val coverage: Boolean = true
) : Params<Testgen.CoverageAndResultsRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.CoverageAndResultsRequest {
        val builder = Testgen.CoverageAndResultsRequest.newBuilder()
            .setCoverage(coverage)
            .setSettingsContext(settingsContextParams.build(remoteMapping))
            .setProjectContext(projectContextParams.build(remoteMapping))
        testFilterParams?.let {
            builder.setTestFilter(it.build(remoteMapping))
        }

        return builder.build()
    }
}

internal data class SourceInfoParams(
    val filePath: String,
    val line: Int
) : Params<Util.SourceInfo> {
    override fun build(remoteMapping: RemoteMapping): Util.SourceInfo {
        return Util.SourceInfo.newBuilder()
            .setLine(line)
            .setFilePath(remoteMapping.convertToRemote(filePath, UTBot.message("filePath.wrong.conversion")))
            .build()
    }
}

data class LineRequestParams(
    val projectRequestParams: Params<Testgen.ProjectRequest>,
    val sourceInfoParams: Params<Util.SourceInfo>
) : Params<Testgen.LineRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.LineRequest {
        return Testgen.LineRequest.newBuilder()
            .setProjectRequest(projectRequestParams.build(remoteMapping))
            .setSourceInfo(sourceInfoParams.build(remoteMapping))
            .build()
    }
}

internal data class FunctionRequestParams(
    val fileRequestParams: Params<Testgen.LineRequest>
) : Params<Testgen.FunctionRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.FunctionRequest {
        return Testgen.FunctionRequest.newBuilder()
            .setLineRequest(fileRequestParams.build(remoteMapping))
            .build()
    }
}

internal data class ClassRequestParams(
    val lineRequestParams: LineRequestParams
) : Params<Testgen.ClassRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ClassRequest {
        return Testgen.ClassRequest.newBuilder()
            .setLineRequest(lineRequestParams.build(remoteMapping))
            .build()
    }
}

internal class AssertionRequestParams(private val lineRequestParams: LineRequestParams) : Params<Testgen.AssertionRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.AssertionRequest {
        return Testgen.AssertionRequest.newBuilder()
            .setLineRequest(lineRequestParams.build(remoteMapping))
            .build()
    }
}

internal data class PredicateInfoParams(
    val validationType: Util.ValidationType,
    val predicate: String,
    val returnValue: String
) : Params<Util.PredicateInfo> {
    override fun build(remoteMapping: RemoteMapping): Util.PredicateInfo {
        return Util.PredicateInfo.newBuilder()
            .setPredicate(predicate)
            .setReturnValue(returnValue)
            .setType(validationType)
            .build()
    }
}

internal class PredicateRequestParams(
    private val lineRequestParams: LineRequestParams,
    private val predicateInfoParams: PredicateInfoParams
) : Params<Testgen.PredicateRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.PredicateRequest {
        return Testgen.PredicateRequest.newBuilder()
            .setLineRequest(lineRequestParams.build(remoteMapping))
            .setPredicateInfo(predicateInfoParams.build(remoteMapping))
            .build()
    }
}

internal class SnippetRequestParams(
    private val projectContextParams: ProjectContextParams,
    private val settingsContextParams: SettingsContextParams,
    val filePath: String
) : Params<Testgen.SnippetRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.SnippetRequest {
        return Testgen.SnippetRequest.newBuilder()
            .setProjectContext(projectContextParams.build(remoteMapping))
            .setSettingsContext(settingsContextParams.build(remoteMapping))
            .setFilePath(remoteMapping.convertToRemote(filePath, UTBot.message("filePath.wrong.conversion")))
            .build()
    }
}
