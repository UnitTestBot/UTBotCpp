package com.huawei.utbot.cpp

import com.intellij.util.io.exists
import java.nio.file.Files
import java.nio.file.Path
import java.nio.file.Paths

enum class Compiler {
    Clang, Gcc
}

fun getBuildCommand(compiler: Compiler, buildDirName: String): String {
    val bear = "/utbot_distr/bear/bin/bear"
    val cmake = "/utbot_distr/install/bin/cmake"
    val clang = "/utbot_distr/install/bin/clang"
    val clangpp = "/utbot_distr/install/bin/clang++"

    return when (compiler) {
        Compiler.Clang -> "export CC=$clang && export CXX=$clangpp && " +
                "mkdir -p $buildDirName && cd $buildDirName && $cmake .. && $bear make -j8"
        Compiler.Gcc -> "export C_INCLUDE_PATH=\"\" && export CC=gcc && export CXX=g++ && " +
                "mkdir -p $buildDirName && cd $buildDirName && $cmake .. && $bear make -j8"
    }
}


fun checkFileExists(path: Path, message: String) {
    assert(path.exists()) { message }
}
