package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.actionSystem.AnActionEvent

fun AnActionEvent.activeProject() = this.project
    ?: error("A project related to action event $this not found")