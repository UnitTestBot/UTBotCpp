package org.utbot.cpp.clion.plugin

import com.intellij.testFramework.fixtures.impl.TempDirTestFixtureImpl
import java.nio.file.Path

/**
 * Implementation of TempDirTestFixture that uses [testsDirectory] as
 * a tempDirectory, and does not delete it on tearDown.
 *
 * Intellij Platform tests are based on files in temp directory, which is provided and managed by TempDirTestFixture.
 * On tearDown, temp directory is deleted.
 * it may be expensive to copy all project files to temporary directory.
 * This class solves the problem, by using [testsDirectory]
 * instead of some generated temp directory.
 */
class TestFixtureProxy(private val testsDirectory: Path) : TempDirTestFixtureImpl() {
    override fun doCreateTempDirectory(): Path {
        return testsDirectory
    }

    // as the directory is not actually temporary, it should not be deleted
    override fun deleteOnTearDown() = false
}
