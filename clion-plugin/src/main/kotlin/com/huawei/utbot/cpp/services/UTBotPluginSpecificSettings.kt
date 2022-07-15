package com.huawei.utbot.cpp.services

import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.State
import com.intellij.openapi.components.Storage
import com.intellij.util.xmlb.XmlSerializerUtil

@State(name = "UTBotPluginSpecificSettings", storages = [Storage("UTBotPluginSpecificSettings.xml")])
data class UTBotPluginSpecificSettings(
    var isFirstLaunch: Boolean = true
) : PersistentStateComponent<UTBotPluginSpecificSettings> {
    override fun getState(): UTBotPluginSpecificSettings {
        return this
    }

    override fun loadState(state: UTBotPluginSpecificSettings) {
        XmlSerializerUtil.copyBean(state, this);
    }
}
