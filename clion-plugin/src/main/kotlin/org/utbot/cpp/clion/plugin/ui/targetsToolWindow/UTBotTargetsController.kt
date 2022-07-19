package org.utbot.cpp.clion.plugin.ui.targetsToolWindow

import com.intellij.openapi.project.Project
import com.intellij.ui.CollectionListModel
import org.utbot.cpp.clion.plugin.utils.getProjectTargetsRequest
import org.utbot.cpp.clion.plugin.client.Client
import org.utbot.cpp.clion.plugin.client.requests.ProjectTargetsRequest
import org.utbot.cpp.clion.plugin.listeners.ConnectionStatus
import org.utbot.cpp.clion.plugin.listeners.UTBotEventsListener
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.settings.UTBotAllSettings
import org.utbot.cpp.clion.plugin.utils.getClient
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.relativize
import org.utbot.cpp.clion.plugin.utils.utbotSettings

class UTBotTargetsController(val project: Project) {
    private val utbotSettings = project.utbotSettings
    private val listModel = CollectionListModel(mutableListOf<UTBotTarget>(UTBotTarget.autoTarget))
    private val client: Client
     get() = project.getClient()
    private val logger = project.logger

    val targets: List<UTBotTarget>
        get() = listModel.toList()

    init {
        requestTargetsFromServer()
        // addTargetPathIfNotPresent(utbotSettings.targetPath)
        connectToEvents()
    }

    fun requestTargetsFromServer() {
        if (!client.isServerAvailable()) {
            logger.error { "Could not request targets from server: server is unavailable!" }
            return
        }
        logger.trace { "Requesting project targets from server!" }
        ProjectTargetsRequest(
            project,
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
                UTBotSettingsChangedListener { settings: UTBotAllSettings ->
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
