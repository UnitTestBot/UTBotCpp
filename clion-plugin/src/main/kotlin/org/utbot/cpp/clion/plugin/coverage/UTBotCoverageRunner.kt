package org.utbot.cpp.clion.plugin.coverage

import com.intellij.coverage.CoverageEngine
import com.intellij.coverage.CoverageRunner
import com.intellij.coverage.CoverageSuite
import com.intellij.openapi.diagnostic.Logger
import com.intellij.rt.coverage.data.LineCoverage
import com.intellij.rt.coverage.data.LineData
import com.intellij.rt.coverage.data.ProjectData
import com.intellij.util.io.exists
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import testsgen.Testgen
import java.io.File
import java.nio.charset.StandardCharsets
import java.nio.file.Files
import java.nio.file.Path

/**
 * This class is used to convert from our representation of coverage to IntelliJ's [ProjectData]
 */
class UTBotCoverageRunner : CoverageRunner() {
    private val log = Logger.getInstance(this::class.java)
    private fun getLineCount(filePath: Path): Int {
        var lineCount: Int
        Files.lines(filePath, StandardCharsets.UTF_8).use { stream -> lineCount = stream.count().toInt() }
        return lineCount
    }

    /**
     * Convert from our coverage representation to IntelliJ Platform representation - [ProjectData]
     */
    override fun loadCoverageData(sessionDataFile: File, baseCoverageSuite: CoverageSuite?): ProjectData? {
        log.debug("loadCoverageData was called!")
        val coveragesList = (baseCoverageSuite as? UTBotCoverageSuite)?.coveragesList
        coveragesList ?: error("Coverage list is empty in loadCoverageData!")
        val projectData = ProjectData()
        var isAnyCoverage = false
        for (simplifiedCovInfo in coveragesList) {
            val filePathFromServer = simplifiedCovInfo.filePath
            if (filePathFromServer.isNotEmpty()) {
                isAnyCoverage = true
                val localFilePath = filePathFromServer.convertFromRemotePathIfNeeded(baseCoverageSuite.project)
                if (!localFilePath.exists()) {
                    log.warn("Skipping $localFilePath in coverage processing as it does not exist!")
                    continue
                }
                val linesCount = getLineCount(localFilePath)
                val lines = arrayOfNulls<LineData>(linesCount)
                val classData = projectData.getOrCreateClassData(provideQualifiedNameForFile(localFilePath.toString()))
                fun processRanges(rangesList: List<Testgen.SourceLine?>, status: Byte) {
                    rangesList.filterNotNull().forEach { sourceLine ->
                        val numberInFile = sourceLine.line - 1
                        if (numberInFile >= linesCount) {
                            log.warn("Skipping $localFilePath:${numberInFile} in coverage processing! Number of lines in file is $linesCount!")
                            return@forEach
                        }
                        val lineData = LineData(sourceLine.line + 1, null)
                        lineData.hits = status.toInt()
                        lineData.setStatus(status)
                        // todo: leave comments what is going on
                        lines[numberInFile + 1] = lineData
                        classData.registerMethodSignature(lineData)
                    }
                }
                processRanges(simplifiedCovInfo.fullCoverageLinesList, LineCoverage.FULL)
                processRanges(simplifiedCovInfo.partialCoverageLinesList, LineCoverage.PARTIAL)
                processRanges(simplifiedCovInfo.noCoverageLinesList, LineCoverage.NONE)
                classData.setLines(lines)
            }
        }
        return if (isAnyCoverage) projectData else null
    }

    override fun getPresentableName(): String = "UTBot: Coverage runner"

    override fun getId(): String = "UTBotCoverageRunner"

    // actually no coverage file exists, but this method must be implemented, see UTBotCoverageFileProvider
    override fun getDataFileExtension(): String = "txt"

    override fun acceptsCoverageEngine(engine: CoverageEngine): Boolean = engine is UTBotCoverageEngine

    companion object {
        fun provideQualifiedNameForFile(absolutePath: String) = absolutePath
    }
}
