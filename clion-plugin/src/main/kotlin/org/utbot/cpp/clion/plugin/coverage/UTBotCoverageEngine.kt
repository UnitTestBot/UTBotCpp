package org.utbot.cpp.clion.plugin.coverage

import com.intellij.coverage.CoverageAnnotator
import com.intellij.coverage.CoverageEngine
import com.intellij.coverage.CoverageFileProvider
import com.intellij.coverage.CoverageRunner
import com.intellij.coverage.CoverageSuite
import com.intellij.coverage.CoverageSuitesBundle
import com.intellij.coverage.SimpleCoverageAnnotator
import com.intellij.execution.configurations.RunConfigurationBase
import com.intellij.execution.configurations.coverage.CoverageEnabledConfiguration
import com.intellij.execution.testframework.AbstractTestProxy
import com.intellij.openapi.module.Module
import com.intellij.openapi.project.Project
import com.intellij.psi.PsiElement
import com.intellij.psi.PsiFile
import org.apache.commons.io.FilenameUtils
import org.utbot.cpp.clion.plugin.utils.isCPPorCFileName
import java.io.File

/**
 * This class must be implemented to create custom [CoverageSuite] - [UTBotCoverageSuite].
 *
 * for additional docs @see [CoverageEngine] in IntelliJ Platform source code
 */
class UTBotCoverageEngine : CoverageEngine() {
    /**
     * This method is not called, when the coverage is processed in CoverageDataManager#coverageGathered(suite).
     *
     * So this method is unused, but needs to be implemented.
     */
    override fun isApplicableTo(conf: RunConfigurationBase<*>) = false

    /**
     * Determines if coverage information should be displayed for given file
     */
    override fun coverageEditorHighlightingApplicableTo(psiFile: PsiFile): Boolean = isCPPorCFileName(psiFile.name)

    /**
     * @return true if we can provide tests that covered a particular line.
     *
     * From information returned from server we can't do that.
     */
    override fun canHavePerTestCoverage(conf: RunConfigurationBase<*>) = false

    /**
     * Not used in our coverage processing but needs to be implemented.
     */
    override fun createCoverageEnabledConfiguration(conf: RunConfigurationBase<*>): CoverageEnabledConfiguration {
        return object : CoverageEnabledConfiguration(conf) {}
    }

    /**
     * Not used in our coverage processing but needs to be implemented.
     */
    override fun createCoverageSuite(
        covRunner: CoverageRunner,
        name: String,
        coverageDataFileProvider: CoverageFileProvider,
        filters: Array<out String>?,
        lastCoverageTimeStamp: Long,
        suiteToMerge: String?,
        coverageByTestEnabled: Boolean,
        tracingEnabled: Boolean,
        trackTestFolders: Boolean,
        project: Project
    ): CoverageSuite? = null

    /**
     * Not used in our coverage processing but needs to be implemented.
     */
    override fun createCoverageSuite(
        covRunner: CoverageRunner,
        name: String,
        coverageDataFileProvider: CoverageFileProvider,
        config: CoverageEnabledConfiguration
    ): CoverageSuite? = null

    /**
     * Not used in our coverage processing but needs to be implemented.
     */
    override fun createEmptyCoverageSuite(coverageRunner: CoverageRunner): CoverageSuite? {
        return null
    }

    /**
     * Not used in our coverage processing but needs to be implemented.
     *
     */
    override fun getCoverageAnnotator(project: Project?): CoverageAnnotator {
        return object : SimpleCoverageAnnotator(project) {}
    }

    /**
     * Coverage is processed only when we receive non-empty response from server,
     * so we don't need to check for empty output directory and recompile project,
     * because server already compiled it and generated coverage.
     */
    override fun recompileProjectAndRerunAction(
        module: Module,
        suite: CoverageSuitesBundle,
        chooseSuiteAction: Runnable
    ) = false

    /**
     * @return qualified names compatible with ProjectData.getClassData(qname)
     *
     * In [UTBotCoverageRunner] we use [UTBotCoverageRunner.provideQualifiedNameForFile] to
     * create ClassData for absolute path of a file. So for compatibility we use it here.
     */
    override fun getQualifiedNames(sourceFile: PsiFile): MutableSet<String> {
        return sourceFile.virtualFile?.path?.let {
            mutableSetOf(UTBotCoverageRunner.provideQualifiedNameForFile(FilenameUtils.separatorsToSystem(it)))
        } ?: mutableSetOf()
    }


    /**
     * output files - something like class files for java files. Useless for c/c++.
     *
     * @return empty set.
     */
    override fun getCorrespondingOutputFiles(
        srcFile: PsiFile,
        module: Module?,
        suite: CoverageSuitesBundle
    ): MutableSet<File> {
        return mutableSetOf()
    }

    /**
     * Checks whether coverage should be shown for file based on [CoverageSuitesBundle].
     * It is used in [CoverageDataManager.applyInformationToEditor].
     *
     * For example, see JavaCoverageEngine: We may not want to show coverage for files in test
     * folders, if suite.isTrackTestFolders == false, we may check it here and return false.
     *
     * For now all files are accepted.
     *
     * @param psiFile Psi file
     * @param suite   Coverage suite
     * @return true
     */
    override fun acceptedByFilters(psiFile: PsiFile, suite: CoverageSuitesBundle): Boolean = true

    /**
     * @return psi references to tests given their names.
     *
     * from call site: test names are gathered from [CoverageEngine.getTestsForLine], then this method is called.
     * We can't find tests that covered specified line, so this method won't be called.
     *
     * It is required for implementation of [CoverageEngine], so we just return empty list.
     *
     * @return empty list.
     */
    override fun findTestsByNames(testNames: Array<out String>, project: Project): MutableList<PsiElement> =
        mutableListOf()

    /**
     * Return the name of a file, which contains traces for a given test.
     *
     * As server returns list of FileCoverages and test results, we can't get the name of a file for a test,
     * so it is not supported.
     *
     * @return null
     */
    override fun getTestMethodName(element: PsiElement, testProxy: AbstractTestProxy): String? = null

    /**
     * If there are multiple coverage engines, then the user will be asked to choose one of them with this text.
     */
    override fun getPresentableText(): String {
        return "UTBot Coverage Engine"
    }
}
