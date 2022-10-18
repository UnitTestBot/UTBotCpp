package org.utbot.cpp.clion.plugin.ui.utbotToolWindow.targetToolWindow

import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import testsgen.Testgen

data class UTBotTarget(val path: String, val name: String, val description: String) {

    constructor(target: Testgen.ProjectTarget, project: Project) : this(
        path =
        if (target.name == autoTarget.name) target.path else target.path.convertFromRemotePathIfNeeded(project)
            .toAbsolutePath()
            .toString(),
        name = target.name,
        description = target.description
    )

    companion object {
        val autoTarget = UTBotTarget(
            path = "/utbot/auto/target/path",
            name = "UTBot: auto",
            description = "Finds any target that contains the code under test"
        )
    }
}
