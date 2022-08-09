package org.utbot.cpp.clion.plugin.coverage

import com.intellij.coverage.CoverageEngine
import com.intellij.coverage.CoverageRunner
import com.intellij.coverage.CoverageSuite
import com.intellij.openapi.diagnostic.Logger
import com.intellij.rt.coverage.data.LineCoverage
import com.intellij.rt.coverage.data.LineData
import com.intellij.rt.coverage.data.ProjectData
import com.intellij.util.io.exists
import java.io.File
import java.nio.charset.StandardCharsets
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.Paths

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
        val fileToCoverageInfo: Map<Path, Coverage> =
            (baseCoverageSuite as? UTBotCoverageSuite)?.coverage ?: return null
        val projectData = ProjectData()
        var isAnyCoverage = false
        for ((file, coverage) in fileToCoverageInfo) {
            isAnyCoverage = true
            if (!file.exists()) {
                log.warn("Skipping $file in coverage processing as it does not exist!")
                continue
            }
            val linesCount = getLineCount(file)
            val lines = arrayOfNulls<LineData>(linesCount)
            val classData = projectData.getOrCreateClassData(provideQualifiedNameForFile(file.toString()))
            fun processLinesBatch(batch: Set<Int>, status: Byte) {
                // assuming: server's coverage lines indexes start from 1
                batch.forEach { lineIdx ->
                    System.err.println("Processing idx : $lineIdx")
                    if (lineIdx > linesCount) {
                        log.warn("Skipping $file:${lineIdx} in coverage processing! Number of lines in file is $linesCount!")
                        return@forEach
                    }
                    val lineData = LineData(lineIdx + 1, null)
                    lineData.hits = status.toInt()
                    lineData.setStatus(status)
                    lines[lineIdx] = lineData
                    classData.registerMethodSignature(lineData)
                }
                classData.setLines(lines)
            }
            processLinesBatch(coverage.fullyCovered, LineCoverage.FULL)
            processLinesBatch(coverage.partiallyCovered, LineCoverage.PARTIAL)
            processLinesBatch(coverage.notCovered, LineCoverage.NONE)
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
