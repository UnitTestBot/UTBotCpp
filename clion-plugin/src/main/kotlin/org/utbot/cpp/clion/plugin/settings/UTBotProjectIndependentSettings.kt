package org.utbot.cpp.clion.plugin.settings

import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage

/**
 * Settings that are the same for all projects
 */
@Service
@State(
    name = "UTBotProjectIndependentSettings",
    storages = [Storage("utbot-project-independent-settings.xml")]
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
    }

    private var myState: State = State()
    override fun getState(): State = myState
    override fun loadState(state: State) {
        myState = state
    }
}