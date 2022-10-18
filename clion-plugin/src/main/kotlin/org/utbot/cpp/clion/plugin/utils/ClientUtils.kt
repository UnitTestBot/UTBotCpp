package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.client.Client
import org.utbot.cpp.clion.plugin.client.ManagedClient
import org.utbot.cpp.clion.plugin.client.logger.ClientLogger

//TODO: remove logger from this file
val Project.logger: ClientLogger get() = this.service()

val AnActionEvent.client: ManagedClient
    get() = this.getRequiredData(CommonDataKeys.PROJECT).client

/**
 * NB: [Client] can be changed when user changes connection settings.
 * Use this function to obtain a reference to a Client and execute request
 * using Client#executeRequestIfNotDisposed.
 */
val Project.client: ManagedClient get() = this.service<ManagedClient>()

fun AnActionEvent.activeProject() = this.project
    ?: error("Project is missing for event: $this")
