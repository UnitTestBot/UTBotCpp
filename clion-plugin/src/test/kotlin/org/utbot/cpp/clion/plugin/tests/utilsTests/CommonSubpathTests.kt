package org.utbot.cpp.clion.plugin.tests.utilsTests

import org.junit.jupiter.api.Assertions
import org.junit.jupiter.params.ParameterizedTest
import org.junit.jupiter.params.provider.Arguments
import org.junit.jupiter.params.provider.MethodSource
import org.utbot.cpp.clion.plugin.utils.getCommonPathFromRoot

import java.nio.file.Paths

internal class CommonSubPathTest {
    companion object {
        @JvmStatic
        fun inputData(): List<Arguments> = listOf(
            Arguments.of("/a/b/c", "/a/b/d/e", "/a/b"),
            Arguments.of("/a/b/d/e", "/a/b/c",  "/a/b"),
            Arguments.of("C:/a/b/g", "C:/a", "C:/a")
        )
    }

    @MethodSource("inputData")
    @ParameterizedTest
    fun doTest(path1: String, path2: String, expected: String) {
        Assertions.assertEquals(
            Paths.get(expected),
            getCommonPathFromRoot(Paths.get(path1), Paths.get(path2))
        )
    }
}
