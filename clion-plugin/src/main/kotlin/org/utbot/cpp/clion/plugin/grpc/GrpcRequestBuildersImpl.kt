package org.utbot.cpp.clion.plugin.grpc

import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow.UTBotTarget
import testsgen.Testgen
import testsgen.Util
import java.nio.file.InvalidPathException
import java.nio.file.Paths


internal data class ProjectContextBuilder(
    val projectName: String,
    val projectPath: String,
    val testDirRelativePath: String,
    val buildDirRelPath: String
) : GrpcRequestBuilder<Testgen.ProjectContext> {
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
            .setBuildDirRelPath(buildDirRelPath)
            .setProjectName(projectName)
            .setProjectPath(remoteMapping.convertToRemote(projectPath, UTBot.message("projectPath.wrong.conversion")))
            .build()
    }
}

internal data class SettingsContextBuilder(
    val generateForStaticFunctions: Boolean,
    val verbose: Boolean,
    val timeoutPerFunction: Int,
    val timeoutPerTest: Int,
    val useDeterministicSearcher: Boolean,
    val useStubs: Boolean
) : GrpcRequestBuilder<Testgen.SettingsContext> {
    override fun build(remoteMapping: RemoteMapping): Testgen.SettingsContext {
        return Testgen.SettingsContext.newBuilder()
            .setVerbose(verbose)
            .setGenerateForStaticFunctions(generateForStaticFunctions)
            .setTimeoutPerFunction(timeoutPerFunction)
            .setTimeoutPerTest(timeoutPerTest)
            .setUseStubs(useStubs)
            .build()
    }
}

internal data class ProjectRequestBuilder(
    val projectContextBuilder: ProjectContextBuilder,
    val settingsContextBuilder: SettingsContextBuilder,
    val sourcePaths: List<String>,
    val synchronizeCode: Boolean,
    val targetPath: String
) : GrpcRequestBuilder<Testgen.ProjectRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ProjectRequest {
        return Testgen.ProjectRequest.newBuilder()
            .setProjectContext(projectContextBuilder.build(remoteMapping))
            .setSettingsContext(settingsContextBuilder.build(remoteMapping))
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
                } else {
                    builder.targetPath = targetPath
                }
            }
            .build()
    }
}

internal data class FolderRequestBuilder(
    val projectRequestBuilder: ProjectRequestBuilder,
    val folderPath: String
) : GrpcRequestBuilder<Testgen.FolderRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.FolderRequest {
        return Testgen.FolderRequest.newBuilder()
            .setFolderPath(remoteMapping.convertToRemote(folderPath, UTBot.message("folderPath.wrong.conversion")))
            .setProjectRequest(projectRequestBuilder.build(remoteMapping))
            .build()
    }
}

internal data class FileRequestBuilder(
    val projectRequestBuilder: ProjectRequestBuilder,
    val filePath: String
) : GrpcRequestBuilder<Testgen.FileRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.FileRequest {
        return Testgen.FileRequest.newBuilder()
            .setProjectRequest(projectRequestBuilder.build(remoteMapping))
            .setFilePath(remoteMapping.convertToRemote(filePath, UTBot.message("filePath.wrong.conversion")))
            .build()
    }
}

internal data class ProjectConfigRequestBuilder(
    val projectContextBuilder: ProjectContextBuilder,
    val configMode: Testgen.ConfigMode,
    val cmakeOptions: List<String>
) : GrpcRequestBuilder<Testgen.ProjectConfigRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ProjectConfigRequest {
        return Testgen.ProjectConfigRequest.newBuilder()
            .setProjectContext(projectContextBuilder.build(remoteMapping))
            .setConfigMode(configMode)
            .addAllCmakeOptions(cmakeOptions)
            .build()
    }
}

internal data class ProjectTargetsParams(
    val projectContextBuilder: ProjectContextBuilder
) : GrpcRequestBuilder<Testgen.ProjectTargetsRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ProjectTargetsRequest {
        return Testgen.ProjectTargetsRequest.newBuilder()
            .setProjectContext(projectContextBuilder.build(remoteMapping))
            .build()
    }
}

internal data class TestFilterBuilder(
    val testFilePath: String,
    val testName: String,
    val testSuite: String
) : GrpcRequestBuilder<Testgen.TestFilter> {
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

internal data class CoverageAndResultsRequestBuilder(
    val projectContextBuilder: ProjectContextBuilder,
    val settingsContextBuilder: SettingsContextBuilder,
    val testFilterBuilder: TestFilterBuilder? = null,
    val coverage: Boolean = true
) : GrpcRequestBuilder<Testgen.CoverageAndResultsRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.CoverageAndResultsRequest {
        val builder = Testgen.CoverageAndResultsRequest.newBuilder()
            .setCoverage(coverage)
            .setSettingsContext(settingsContextBuilder.build(remoteMapping))
            .setProjectContext(projectContextBuilder.build(remoteMapping))
        testFilterBuilder?.let {
            builder.setTestFilter(it.build(remoteMapping))
        }

        return builder.build()
    }
}

internal data class SourceInfoParams(
    val filePath: String,
    val line: Int
) : GrpcRequestBuilder<Util.SourceInfo> {
    override fun build(remoteMapping: RemoteMapping): Util.SourceInfo {
        return Util.SourceInfo.newBuilder()
            .setLine(line)
            .setFilePath(remoteMapping.convertToRemote(filePath, UTBot.message("filePath.wrong.conversion")))
            .build()
    }
}

data class LineRequestBuilder(
    val projectRequestParams: GrpcRequestBuilder<Testgen.ProjectRequest>,
    val sourceInfoParams: GrpcRequestBuilder<Util.SourceInfo>
) : GrpcRequestBuilder<Testgen.LineRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.LineRequest {
        return Testgen.LineRequest.newBuilder()
            .setProjectRequest(projectRequestParams.build(remoteMapping))
            .setSourceInfo(sourceInfoParams.build(remoteMapping))
            .build()
    }
}

internal data class FunctionRequestBuilder(
    val fileRequestParams: GrpcRequestBuilder<Testgen.LineRequest>
) : GrpcRequestBuilder<Testgen.FunctionRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.FunctionRequest {
        return Testgen.FunctionRequest.newBuilder()
            .setLineRequest(fileRequestParams.build(remoteMapping))
            .build()
    }
}

internal data class ClassRequestParams(
    val lineRequestBuilder: LineRequestBuilder
) : GrpcRequestBuilder<Testgen.ClassRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.ClassRequest {
        return Testgen.ClassRequest.newBuilder()
            .setLineRequest(lineRequestBuilder.build(remoteMapping))
            .build()
    }
}

internal class AssertionRequestBuilder(private val lineRequestBuilder: LineRequestBuilder) :
    GrpcRequestBuilder<Testgen.AssertionRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.AssertionRequest {
        return Testgen.AssertionRequest.newBuilder()
            .setLineRequest(lineRequestBuilder.build(remoteMapping))
            .build()
    }
}

internal data class PredicateInfoParams(
    val validationType: Util.ValidationType,
    val predicate: String,
    val returnValue: String
) : GrpcRequestBuilder<Util.PredicateInfo> {
    override fun build(remoteMapping: RemoteMapping): Util.PredicateInfo {
        return Util.PredicateInfo.newBuilder()
            .setPredicate(predicate)
            .setReturnValue(returnValue)
            .setType(validationType)
            .build()
    }
}

internal class PredicateRequestBuilder(
    private val lineRequestBuilder: LineRequestBuilder,
    private val predicateInfoParams: PredicateInfoParams
) : GrpcRequestBuilder<Testgen.PredicateRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.PredicateRequest {
        return Testgen.PredicateRequest.newBuilder()
            .setLineRequest(lineRequestBuilder.build(remoteMapping))
            .setPredicateInfo(predicateInfoParams.build(remoteMapping))
            .build()
    }
}

internal class SnippetRequestBuilder(
    private val projectContextBuilder: ProjectContextBuilder,
    private val settingsContextBuilder: SettingsContextBuilder,
    val filePath: String
) : GrpcRequestBuilder<Testgen.SnippetRequest> {
    override fun build(remoteMapping: RemoteMapping): Testgen.SnippetRequest {
        return Testgen.SnippetRequest.newBuilder()
            .setProjectContext(projectContextBuilder.build(remoteMapping))
            .setSettingsContext(settingsContextBuilder.build(remoteMapping))
            .setFilePath(remoteMapping.convertToRemote(filePath, UTBot.message("filePath.wrong.conversion")))
            .build()
    }
}
