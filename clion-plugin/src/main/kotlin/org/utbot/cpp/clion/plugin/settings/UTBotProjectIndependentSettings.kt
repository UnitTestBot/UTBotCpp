package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage
import org.utbot.cpp.clion.plugin.listeners.ConnectionSettingsListener

/**
 * Settings that are the same for all projects
 */
@Service
@State(
    name = "UtBotGlobalSettings",
    storages = [Storage("utbot-global-settings.xml")]
)
class UTBotProjectIndependentSettings : PersistentStateComponent<UTBotProjectIndependentSettings.State> {
    data class State(
        var port: Int = UTBotAllProjectSettings.DEFAULT_PORT,
        var serverName: String = UTBotAllProjectSettings.DEFAULT_HOST,
    ) {
        fun fromSettingsModel(model: UTBotSettingsModel) {
            port = model.globalSettings.port
            serverName = model.globalSettings.serverName
        }

        fun fireConnectionSettingsChanged() {
            ApplicationManager.getApplication().messageBus.syncPublisher(ConnectionSettingsListener.TOPIC)
                .connectionSettingsChanged(port, serverName)
        }
    }

    private var myState: State = State()
    override fun getState(): State = myState
    override fun loadState(state: State) {
        myState = state
    }

}