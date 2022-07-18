package org.utbot.cpp.clion.plugin.ui.wizard.steps

import com.intellij.openapi.project.Project
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.bindText
import com.intellij.ui.dsl.builder.columns
import com.intellij.ui.dsl.builder.panel
import org.utbot.cpp.clion.plugin.services.UTBotSettings
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizardStep
import org.utbot.cpp.clion.plugin.utils.commandLineEditor

class BuildOptionsStep(private val project: Project, private val settingsModel: UTBotSettings.State) :
    UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/build_dir.html")
        panel {
            row("Relative Path to Build Folder") {
                textField().bindText(settingsModel::buildDirRelativePath).columns(COLUMNS_LARGE)
            }
        }.addToUI()
        addHtml("media/cmake_options.html")
        panel {
            row {
                commandLineEditor({ settingsModel.cmakeOptions.joinToString(" ") },
                    { value: String -> settingsModel.cmakeOptions = value.split(" ") })
            }
        }.addToUI()
    }
}
