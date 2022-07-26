package org.utbot.cpp.clion.plugin.ui.targetsToolWindow

import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import testsgen.Testgen

data class UTBotTarget(val path: String, val name: String, val description: String) {
    constructor(target: Testgen.ProjectTarget, project: Project) : this(
        if (target.name == autoTarget.name) target.path else target.path.convertFromRemotePathIfNeeded(project),
        target.name,
        target.description
    )

    companion object {
        val autoTarget = UTBotTarget(
            "/utbot/auto/target/path",
            "UTBot: auto",
            "Finds any target that contains the code under testing"
        )
    }
}