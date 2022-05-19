package com.huawei.utbot.cpp

import java.nio.file.Path

abstract class CppCompiler {
    abstract val name: String

    protected val bearPath = "/utbot_distr/bear/bin/bear"
    protected val cmakePath = "/utbot_distr/install/bin/cmake"
    private val logger = com.intellij.openapi.diagnostic.Logger.getInstance(this.javaClass)

    abstract fun produceBuildCommand(buildDirName: String): String
    fun buildProject(projectPath: Path, buildDirName: String) {
        val buildCommand = produceBuildCommand(buildDirName)
        logger.trace("Building the project with compiler: $name, and build directory name: $buildDirName")
        logger.trace("BUILD COMMAND: $buildCommand")
        ProcessBuilder("bash", "-c", buildCommand)
            .directory(projectPath.toFile())
            .start()
            .waitFor()

        logger.trace("BUILDING FINISHED!")
        projectPath.resolve(buildDirName).assertFileOrDirExists("Build directory after building project does not exist!")
    }
}

object Clang: CppCompiler() {
    override val name: String
        get() = "Clang"

    private const val clangPath = "/utbot_distr/install/bin/clang"
    private const val clangppPath = "/utbot_distr/install/bin/clang++"

    override fun produceBuildCommand(buildDirName: String) = "export CC=$clangPath && export CXX=$clangppPath && " +
            "mkdir -p $buildDirName && cd $buildDirName && $cmakePath .. && $bearPath make -j8"
}

object Gcc: CppCompiler() {
    override val name: String
        get() = "Gcc"

    override fun produceBuildCommand(buildDirName: String) = "export C_INCLUDE_PATH=\"\" && export CC=gcc && export CXX=g++ && " +
            "mkdir -p $buildDirName && cd $buildDirName && $cmakePath .. && $bearPath make -j8"
}
