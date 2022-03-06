package com.huawei.utbot.cpp.ui

import com.huawei.utbot.cpp.actions.utils.getProjectTargetsRequest
import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.utils.relativize
import com.huawei.utbot.cpp.messaging.UTBotSettingsChangedListener
import com.huawei.utbot.cpp.services.UTBotSettings
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.ui.CollectionListModel
import com.huawei.utbot.cpp.messaging.ConnectionStatus
import com.huawei.utbot.cpp.messaging.UTBotEventsListener
import com.huawei.utbot.cpp.models.UTBotTarget


class UTBotTargetsController(val project: Project) {
    private val utbotSettings = project.service<UTBotSettings>()
    private val listModel = CollectionListModel(mutableListOf<UTBotTarget>(UTBotTarget.autoTarget))
    private val client = project.service<Client>()

    init {
        requestTargetsFromServer()
        addTargetPathIfNotPresent(utbotSettings.targetPath)
        connectToEvents()
    }

    private fun requestTargetsFromServer() {
        if (client.isServerAvailable()) {
            client.requestProjectTargetsAndProcess(getProjectTargetsRequest(project)) { targetsResponse ->
                ApplicationManager.getApplication().invokeLater {
                    listModel.apply {
                        val oldTargetList = toList()
                        oldTargetList.addAll(
                            targetsResponse.targetsList.map { projectTarget ->
                                UTBotTarget(projectTarget, project)
                            })
                        listModel.replaceAll(oldTargetList.distinct())
                    }
                }
            }
        }
    }

    private fun addTargetPathIfNotPresent(possiblyNewTargetPath: String) {
        listModel.apply {
            toList().find { utbotTarget -> utbotTarget.path == possiblyNewTargetPath } ?: add(
                UTBotTarget(
                    possiblyNewTargetPath,
                    "custom target",
                    relativize(utbotSettings.projectPath, possiblyNewTargetPath)
                )
            )
        }
    }

    fun createTargetsToolWindow(): UTBotTargetsToolWindow {
        return UTBotTargetsToolWindow(listModel, this)
    }

    fun selectionChanged(index: Int) {
        // when user selects target update model
        if (index in 0 until listModel.size) {
            utbotSettings.targetPath = listModel.getElementAt(index).path
        }
    }

    private fun connectToEvents() {
        project.messageBus.connect().also { connection ->
            // if user specifies some custom target path in settings, it will be added if not already present
            connection.subscribe(
                UTBotSettingsChangedListener.TOPIC,
                UTBotSettingsChangedListener { settings: UTBotSettings ->
                    val possiblyNewTargetPath = settings.targetPath
                    addTargetPathIfNotPresent(possiblyNewTargetPath)
                })
            // when we reconnected to server, the targets should be updated, so we request them from server
            connection.subscribe(
                UTBotEventsListener.CONNECTION_CHANGED_TOPIC,
                object : UTBotEventsListener {
                    override fun onConnectionChange(oldStatus: ConnectionStatus, newStatus: ConnectionStatus) {
                        if (newStatus != oldStatus && newStatus == ConnectionStatus.CONNECTED) {
                            requestTargetsFromServer()
                        }
                    }
                }
            )
        }
    }
}
