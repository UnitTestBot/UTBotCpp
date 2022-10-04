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
 * Facade that creates Params<X>, where X is the grpc message
 */
class ParamsBuilder(
    val project: Project
) {
    private fun buildProjectContextParams(): Params<Testgen.ProjectContext> {
        return ProjectContextParams(
            project.name,
            project.path,
            project.settings.storedSettings.testDirRelativePath,
            project.settings.storedSettings.buildDirRelativePath
        )
    }

    private fun buildSettingsContextParams(): Params<Testgen.SettingsContext> {
        return project.settings.storedSettings.let {
            SettingsContextParams(
                it.generateForStaticFunctions,
                it.verbose,
                it.timeoutPerFunction,
                it.timeoutPerTest,
                it.useDeterministicSearcher,
                it.useStubs
            )
        }
    }

    fun buildProjectRequestParams(): Params<Testgen.ProjectRequest> {
        val settings = project.settings.storedSettings
        val sourcePaths = settings.sourceDirs.toList()
        return ProjectRequestParams(
            buildProjectContextParams() as ProjectContextParams,
            buildSettingsContextParams() as SettingsContextParams,
            sourcePaths,
            project.settings.isRemoteScenario,
            settings.targetPath
        )
    }


    fun buildLineRequestBuilder(lineNumber: Int, filePath: String): Params<Testgen.LineRequest> {
        return LineRequestParams(
            buildProjectRequestParams(),
            SourceInfoParams(filePath, lineNumber)
        )
    }

    fun buildFunctionRequestParams(filePath: String, lineNumber: Int): Params<Testgen.FunctionRequest> {
        return FunctionRequestParams(
            buildLineRequestBuilder(lineNumber, filePath)
        )
    }

    fun buildFolderRequestParams(folderPath: String): Params<Testgen.FolderRequest> {
        return FolderRequestParams(
            buildProjectRequestParams() as ProjectRequestParams,
            folderPath
        )
    }

    fun buildFileRequestParams(filePath: String): Params<Testgen.FileRequest> {
        return FileRequestParams(
            buildProjectRequestParams() as ProjectRequestParams,
            filePath
        )
    }

    fun buildProjectConfigRequestParams(configMode: Testgen.ConfigMode): Params<Testgen.ProjectConfigRequest> {
        return ProjectConfigRequestParams(
            buildProjectContextParams() as ProjectContextParams,
            configMode,
            project.settings.storedSettings.cmakeOptions.split(" ")
        )
    }

    fun buildProjectTargetsParams(): Params<Testgen.ProjectTargetsRequest> {
        return ProjectTargetsParams(buildProjectContextParams() as ProjectContextParams)
    }

    /**
     * if [element] is null return params that correspond for running all tests
     */
    fun buildCoverageAndResultsRequestParams(element: PsiElement? = null): Params<Testgen.CoverageAndResultsRequest> {
        return CoverageAndResultsRequestParams(
            buildProjectContextParams() as ProjectContextParams,
            buildSettingsContextParams() as SettingsContextParams,
            element?.let { buildTestFilterParams(it) as TestFilterParams },
            coverage = true
        )
    }

    private fun buildTestFilterParams(element: PsiElement): Params<Testgen.TestFilter> {
        val (testName: String, testSuite: String) = TestNameAndTestSuite.create(element)
        val testFilePath = element.containingFile.virtualFile.localPath.toString()
        return TestFilterParams(
            testFilePath,
            testName,
            testSuite
        )
    }

    fun buildClassRequestParams(filePath: String, lineNumber: Int): Params<Testgen.ClassRequest> {
        return ClassRequestParams(buildLineRequestBuilder(lineNumber, filePath) as LineRequestParams)
    }

    fun buildAssertionRequestParams(lineNumber: Int, filePath: String): Params<Testgen.AssertionRequest> {
        return AssertionRequestParams(
            buildLineRequestBuilder(lineNumber, filePath) as LineRequestParams
        )
    }

    fun buildPredicateRequestParams(
        comparisonOperator: String,
        validationType: Util.ValidationType,
        valueToCompare: String,
        lineNumber: Int,
        filePath: String
    ): Params<Testgen.PredicateRequest> {
        return PredicateRequestParams(
            buildLineRequestBuilder(lineNumber, filePath) as LineRequestParams,
            PredicateInfoParams(validationType, comparisonOperator, valueToCompare)
        )
    }

    fun buildSnippetRequestParams(filePath: String): Params<Testgen.SnippetRequest> {
        return SnippetRequestParams(
            buildProjectContextParams() as ProjectContextParams,
            buildSettingsContextParams() as SettingsContextParams,
            filePath
        )
    }
}
