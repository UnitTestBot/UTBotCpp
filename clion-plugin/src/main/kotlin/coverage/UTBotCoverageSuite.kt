package coverage

import com.intellij.coverage.BaseCoverageSuite
import com.intellij.coverage.CoverageEngine
import com.intellij.coverage.CoverageLogger
import com.intellij.coverage.CoverageRunner
import com.intellij.openapi.project.Project
import com.intellij.rt.coverage.data.ProjectData
import testsgen.Testgen
import java.io.File
import java.util.*
import java.util.concurrent.TimeUnit

/**
 * Stores coverage information and coverage settings. In current implementation coverage settings are unused.
 *
 * @param covLists - coverage information returned from server.
 */
class UTBotCoverageSuite(
    coverageEngine: UTBotCoverageEngine,
    covLists: List<Testgen.FileCoverageSimplified>? = null,
    name: String? = null,
    utbotFileProvider: UTBotCoverageFileProvider? = UTBotCoverageFileProvider(),
    lastCoverageTimeStamp: Long = Date().time,
    coverageByTestEnabled: Boolean = false,
    tracingEnabled: Boolean = false,
    trackTestFolders: Boolean = false,
    coverageRunner: CoverageRunner? = null,
    project: Project,
) : BaseCoverageSuite(
    name, utbotFileProvider, lastCoverageTimeStamp, coverageByTestEnabled, tracingEnabled, trackTestFolders,
    coverageRunner, project
) {

    val covEngine = coverageEngine
    val covRunner = coverageRunner
    val coveragesList: List<Testgen.FileCoverageSimplified>? = covLists

    override fun getCoverageEngine(): CoverageEngine {
        return covEngine
    }

    /**
     * in parent's implementation this method deletes coverage file returned from [CoverageFileProvider].
     * As we don't use any files, it should do nothing.
     */
    override fun deleteCachedCoverageData() {}

    /**
     * It is called when contents of a file are changed for externally added suite.
     * If contents are changed then our coverage data is outdated, so just let it be null.
     */
    override fun restoreCoverageData() {
        coverageData = null
    }

    /**
     * in parent's implementation this method checks existence of the coverage file returned from [CoverageFileProvider].
     * As we don't use any files, the part with file was deleted.
     */
    override fun loadProjectInfo(): ProjectData? {
        val startNs = System.nanoTime()
        val projectData = covRunner?.loadCoverageData(File(""), this)
        val timeMs = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startNs)
        if (projectData != null) {
            CoverageLogger.logReportLoading(project, covRunner!!, timeMs, projectData.classesNumber)
        }
        return projectData
    }
}
