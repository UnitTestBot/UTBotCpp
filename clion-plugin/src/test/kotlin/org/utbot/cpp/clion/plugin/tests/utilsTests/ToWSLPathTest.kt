package org.utbot.cpp.clion.plugin.tests.utilsTests

import org.junit.jupiter.api.Assertions
import org.junit.jupiter.params.ParameterizedTest
import org.junit.jupiter.params.provider.Arguments
import org.junit.jupiter.params.provider.MethodSource
import org.utbot.cpp.clion.plugin.utils.toWSLPathOnWindows

class ToWSLPathTest {
    companion object {
        @JvmStatic
        fun inputData(): List<Arguments> = listOf(
            Arguments.of("D:\\a\\b\\c", "/mnt/a/b/c"),
            Arguments.of("a:\\\\a\\\\b\\\\c", "/mnt/a/b/c"),
            Arguments.of("\\a\\b\\c", "/mnt/a/b/c"),
        )
    }

    @MethodSource("inputData")
    @ParameterizedTest
    fun doTest(path: String, expected: String) {
        Assertions.assertEquals(
            expected,
            toWSLPathOnWindows(path)
        )
    }
}
