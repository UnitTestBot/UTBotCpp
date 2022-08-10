package org.utbot.cpp.clion.plugin.coverage

import java.nio.file.Path


data class Coverage(
    val fullyCovered: Set<Int> = setOf(),
    val partiallyCovered: Set<Int> = setOf(),
    val notCovered: Set<Int> = setOf()
)
