package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.CommonDataKeys
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.client.Client
import org.utbot.cpp.clion.plugin.client.ClientManager
import org.utbot.cpp.clion.plugin.client.logger.ClientLogger

//TODO: remove logger from this file
val Project.logger: ClientLogger get() = this.service()

val AnActionEvent.currentClient: Client
    get() = this.getRequiredData(CommonDataKeys.PROJECT).getCurrentClient()

/**
 * NB: [Client] can be changed when user changes connection settings.
 * Use this function to obtain a reference to a Client and execute request
 * using Client#executeRequestIfNotDisposed.
 */
fun Project.getCurrentClient(): Client = this.service<ClientManager>().client

fun AnActionEvent.activeProject() = this.project
    ?: error("A project related to action event $this not found")
