package org.utbot.cpp.clion.plugin.coverage

import com.intellij.coverage.CoverageFileProvider

/**
 * IntelliJ Platform api assumes that after execution coverage information
 * was saved to a file. In our case we retrieve it from server, we
 * don't need this class but for compatibility with api this class must be implemented.
 */
class UTBotCoverageFileProvider : CoverageFileProvider {
    override fun getCoverageDataFilePath(): String? = null

    override fun ensureFileExists(): Boolean = true

    override fun isValid(): Boolean = true
}
