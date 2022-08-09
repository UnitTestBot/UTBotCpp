package org.utbot.cpp.clion.plugin.coverage

data class Coverage(
    val fullyCovered: MutableSet<Int> = mutableSetOf(),
    val partiallyCovered: MutableSet<Int> = mutableSetOf(),
    val notCovered: MutableSet<Int> = mutableSetOf()
)

