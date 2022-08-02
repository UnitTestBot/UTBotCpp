package org.utbot.cpp.clion.plugin.ui.targetsToolWindow

import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project
import com.intellij.ui.CollectionListModel
import org.utbot.cpp.clion.plugin.client.requests.ProjectTargetsRequest
import org.utbot.cpp.clion.plugin.grpc.getProjectTargetsGrpcRequest
import org.utbot.cpp.clion.plugin.listeners.ConnectionStatus
import org.utbot.cpp.clion.plugin.listeners.UTBotEventsListener
import org.utbot.cpp.clion.plugin.listeners.UTBotSettingsChangedListener
import org.utbot.cpp.clion.plugin.settings.UTBotAllProjectSettings
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.getCurrentClient
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.relativize
import testsgen.Testgen

@Service
class UTBotTargetsController(val project: Project) {
    private val settings: UTBotAllProjectSettings
        get() = project.settings

    private val listModel = CollectionListModel(mutableListOf<UTBotTarget>())
    private val logger get() = project.logger
    val targetsToolWindow: UTBotTargetsToolWindow by lazy { UTBotTargetsToolWindow(listModel, this) }

    val targets: List<UTBotTarget>
        get() = listModel.toList()

    init {
        requestTargetsFromServer()
        connectToEvents()
    }

    fun requestTargetsFromServer() {
        val currentClient = project.getCurrentClient()

        invokeOnEdt {
            listModel.removeAll()
            targetsToolWindow.setBusy(true)
        }
        ProjectTargetsRequest(
            project,
            getProjectTargetsGrpcRequest(project),
            processTargets = { targetsResponse: Testgen.ProjectTargetsResponse ->
                invokeOnEdt {
                    targetsToolWindow.setBusy(false)

                    listModel.apply {
                        listModel.replaceAll(
                            targetsResponse.targetsList.map { projectTarget ->
                                UTBotTarget(projectTarget, project)
                            })
                    }
                }
            },
            onError = {
                invokeOnEdt {
                    targetsToolWindow.setBusy(false)
                }
            }).let { targetsRequest ->
            if (!currentClient.isServerAvailable()) {
                logger.error { "Could not request targets from server: server is unavailable!" }
                invokeOnEdt {
                    targetsToolWindow.setBusy(false)
                }
                return
            }
            logger.trace { "Requesting project targets from server!" }
            currentClient.executeRequestIfNotDisposed(targetsRequest)
        }
    }

    private fun addTargetPathIfNotPresent(possiblyNewTargetPath: String) {
        listModel.apply {
            toList().find { utbotTarget -> utbotTarget.path == possiblyNewTargetPath } ?: add(
                UTBotTarget(
                    possiblyNewTargetPath,
                    "custom target",
                    relativize(settings.projectPath, possiblyNewTargetPath)
                )
            )
        }
    }

    fun selectionChanged(selectedTarget: UTBotTarget) {
        // when user selects target update model
        settings.storedSettings.targetPath = selectedTarget.path
    }

    fun setTargetByName(targetName: String) {
        val target = targets.find { it.name == targetName } ?: error("No such target!")
        settings.storedSettings.targetPath = target.path
    }

    private fun connectToEvents() {
        project.messageBus.connect().also { connection ->
            // if user specifies some custom target path in settings, it will be added if not already present
            connection.subscribe(
                UTBotSettingsChangedListener.TOPIC,
                UTBotSettingsChangedListener {
                    // todo: remove custom target
                    // val possiblyNewTargetPath = settings.storedSettings.targetPath
                    // addTargetPathIfNotPresent(possiblyNewTargetPath)
                })
            // when we reconnected to server, the targets should be updated, so we request them from server
            connection.subscribe(
                UTBotEventsListener.CONNECTION_CHANGED_TOPIC,
                object : UTBotEventsListener {
                    override fun onConnectionChange(oldStatus: ConnectionStatus, newStatus: ConnectionStatus) {
                        if (newStatus != oldStatus) {
                            requestTargetsFromServer()
                        }
                    }
                }
            )
        }
    }
}
