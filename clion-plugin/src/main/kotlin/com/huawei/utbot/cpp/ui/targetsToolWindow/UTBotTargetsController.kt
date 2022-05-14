package com.huawei.utbot.cpp.ui.targetsToolWindow

import com.huawei.utbot.cpp.actions.utils.getProjectTargetsRequest
import com.huawei.utbot.cpp.client.Client
import com.huawei.utbot.cpp.client.Requests.ProjectTargetsRequest
import com.huawei.utbot.cpp.messaging.ConnectionStatus
import com.huawei.utbot.cpp.messaging.UTBotEventsListener
import com.huawei.utbot.cpp.messaging.UTBotSettingsChangedListener
import com.huawei.utbot.cpp.models.UTBotTarget
import com.huawei.utbot.cpp.services.UTBotSettings
import com.huawei.utbot.cpp.utils.getClient
import com.huawei.utbot.cpp.utils.invokeOnEdt
import com.huawei.utbot.cpp.utils.relativize
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project
import com.intellij.ui.CollectionListModel
import org.tinylog.kotlin.Logger

class UTBotTargetsController(val project: Project) {
    private val utbotSettings = project.service<UTBotSettings>()
    private val listModel = CollectionListModel(mutableListOf<UTBotTarget>(UTBotTarget.autoTarget))
    private val client: Client
     get() = project.getClient()

    val targets: List<UTBotTarget>
        get() = listModel.toList()

    init {
        requestTargetsFromServer()
        addTargetPathIfNotPresent(utbotSettings.targetPath)
        connectToEvents()
    }

    fun requestTargetsFromServer() {
        if (!client.isServerAvailable()) {
            Logger.error("Could not request targets from server: server is unavailable!")
            return
        }
        Logger.trace("Requesting project targets from server!")
        ProjectTargetsRequest(
            getProjectTargetsRequest(project),
        ) { targetsResponse ->
            invokeOnEdt {
                listModel.apply {
                    val oldTargetList = toList()
                    oldTargetList.addAll(
                        targetsResponse.targetsList.map { projectTarget ->
                            UTBotTarget(projectTarget, project)
                        })
                    listModel.replaceAll(oldTargetList.distinct())
                }
            }
        }.let {
            client.executeRequest(it)
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

    fun selectionChanged(selectedTarget: UTBotTarget) {
        // when user selects target update model
        utbotSettings.targetPath = selectedTarget.path
    }

    fun setTargetByName(targetName: String) {
        val target = targets.find { it.name == targetName } ?: error("No such target!")
        utbotSettings.targetPath = target.path
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
