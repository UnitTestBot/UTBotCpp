package com.huawei.utbot.cpp.services

import com.intellij.openapi.components.PersistentStateComponent
import com.intellij.openapi.components.State
import com.intellij.openapi.components.service
import com.intellij.openapi.diagnostic.Logger
import com.intellij.openapi.project.Project
import com.intellij.util.xmlb.XmlSerializerUtil

@State(name = "UTBotGeneratorSettings")
data class GeneratorSettings(
    @com.intellij.util.xmlb.annotations.Transient
    val project: Project? = null,
    var generateForStaticFunctions: Boolean = true,
    var useStubs: Boolean = true,
    var useDeterministicSearcher: Boolean = true,
    var verbose: Boolean = false,
    var timeoutPerFunction: Int = 0,
    var timeoutPerTest: Int = 30
) : PersistentStateComponent<GeneratorSettings> {

    @com.intellij.util.xmlb.annotations.Transient
    val logger = Logger.getInstance(this::class.java)

    init {
        logger.info("constructor was called: project == $project")
    }

    override fun getState(): GeneratorSettings {
        logger.info("getState was called: $this")
        return this
    }

    override fun loadState(state: GeneratorSettings) {
        logger.info("loadState was called: $state \n $this")
        XmlSerializerUtil.copyBean(state, this);
    }

    companion object {
        @JvmStatic
        fun instance(project: Project): GeneratorSettings {
            return project.service()
        }
    }
}
