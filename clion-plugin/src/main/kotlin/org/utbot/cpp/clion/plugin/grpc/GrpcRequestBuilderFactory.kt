package org.utbot.cpp.clion.plugin.grpc

import com.intellij.openapi.project.Project
import com.intellij.psi.PsiElement
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.testsResults.TestNameAndTestSuite
import org.utbot.cpp.clion.plugin.utils.localPath
import org.utbot.cpp.clion.plugin.utils.path
import testsgen.Testgen
import testsgen.Util

/**
 * A facade that provides builders for grpc generated requests aka GrpcRequestBuilder<G>,
 * where G is a grpc generated message
 */
class GrpcRequestBuilderFactory(
    val project: Project
) {
    fun createProjectContextBuilder(): GrpcRequestBuilder<Testgen.ProjectContext> {
        return ProjectContextBuilder(
            project.name,
            project.path,
            project.settings.storedSettings.testDirRelativePath,
            project.settings.storedSettings.buildDirRelativePath
        )
    }

    fun createSettingsContextBuilder(): GrpcRequestBuilder<Testgen.SettingsContext> {
        return project.settings.storedSettings.let {
            SettingsContextBuilder(
                it.generateForStaticFunctions,
                it.verbose,
                it.timeoutPerFunction,
                it.timeoutPerTest,
                it.useDeterministicSearcher,
                it.useStubs
            )
        }
    }

    fun createProjectRequestBuilder(): GrpcRequestBuilder<Testgen.ProjectRequest> {
        val settings = project.settings.storedSettings
        val sourcePaths = settings.sourceDirs.toList()
        return ProjectRequestBuilder(
            createProjectContextBuilder() as ProjectContextBuilder,
            createSettingsContextBuilder() as SettingsContextBuilder,
            sourcePaths,
            project.settings.isRemoteScenario,
            settings.targetPath
        )
    }

    fun createLineRequestBuilder(lineNumber: Int, filePath: String): GrpcRequestBuilder<Testgen.LineRequest> {
        return LineRequestBuilder(
            createProjectRequestBuilder(),
            SourceInfoParams(filePath, lineNumber)
        )
    }

    fun createFunctionRequestBuilder(filePath: String, lineNumber: Int): GrpcRequestBuilder<Testgen.FunctionRequest> {
        return FunctionRequestBuilder(
            createLineRequestBuilder(lineNumber, filePath)
        )
    }

    fun createFolderRequestBuilder(folderPath: String): GrpcRequestBuilder<Testgen.FolderRequest> {
        return FolderRequestBuilder(
            createProjectRequestBuilder() as ProjectRequestBuilder,
            folderPath
        )
    }

    fun createFileRequestBuilder(filePath: String): GrpcRequestBuilder<Testgen.FileRequest> {
        return FileRequestBuilder(
            createProjectRequestBuilder() as ProjectRequestBuilder,
            filePath
        )
    }

    fun createProjectConfigRequestBuilder(configMode: Testgen.ConfigMode): GrpcRequestBuilder<Testgen.ProjectConfigRequest> {
        return ProjectConfigRequestBuilder(
            createProjectContextBuilder() as ProjectContextBuilder,
            configMode,
            project.settings.storedSettings.cmakeOptions.split(" ")
        )
    }

    fun createProjectTargetsRequestBuilder(): GrpcRequestBuilder<Testgen.ProjectTargetsRequest> {
        return ProjectTargetsParams(createProjectContextBuilder() as ProjectContextBuilder)
    }

    /**
     * if [element] is null return params that correspond for running all tests
     */
    fun createCovAndResulstsRequestBuilder(element: PsiElement? = null): GrpcRequestBuilder<Testgen.CoverageAndResultsRequest> {
        return CoverageAndResultsRequestBuilder(
            createProjectContextBuilder() as ProjectContextBuilder,
            createSettingsContextBuilder() as SettingsContextBuilder,
            element?.let { createTestFilterBuilder(it) as TestFilterBuilder },
            coverage = true
        )
    }

    private fun createTestFilterBuilder(element: PsiElement): GrpcRequestBuilder<Testgen.TestFilter> {
        val (testName: String, testSuite: String) = TestNameAndTestSuite.create(element)
        val testFilePath = element.containingFile.virtualFile.localPath.toString()
        return TestFilterBuilder(
            testFilePath,
            testName,
            testSuite
        )
    }

    fun createClassRequestBuilder(filePath: String, lineNumber: Int): GrpcRequestBuilder<Testgen.ClassRequest> {
        return ClassRequestParams(createLineRequestBuilder(lineNumber, filePath) as LineRequestBuilder)
    }

    fun createAssertionRequestBuilder(lineNumber: Int, filePath: String): GrpcRequestBuilder<Testgen.AssertionRequest> {
        return AssertionRequestBuilder(
            createLineRequestBuilder(lineNumber, filePath) as LineRequestBuilder
        )
    }

    fun createPredicateRequestBuilder(
        comparisonOperator: String,
        validationType: Util.ValidationType,
        valueToCompare: String,
        lineNumber: Int,
        filePath: String
    ): GrpcRequestBuilder<Testgen.PredicateRequest> {
        return PredicateRequestBuilder(
            createLineRequestBuilder(lineNumber, filePath) as LineRequestBuilder,
            PredicateInfoParams(validationType, comparisonOperator, valueToCompare)
        )
    }

    fun createSnippetRequestBuilder(filePath: String): GrpcRequestBuilder<Testgen.SnippetRequest> {
        return SnippetRequestBuilder(
            createProjectContextBuilder() as ProjectContextBuilder,
            createSettingsContextBuilder() as SettingsContextBuilder,
            filePath
        )
    }
}
