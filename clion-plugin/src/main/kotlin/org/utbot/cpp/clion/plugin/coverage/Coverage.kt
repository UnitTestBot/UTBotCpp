package org.utbot.cpp.clion.plugin.coverage

data class Coverage(
    val fullyCovered: Set<Int> = setOf(),
    val partiallyCovered: Set<Int> = setOf(),
    val notCovered: Set<Int> = setOf()
)
