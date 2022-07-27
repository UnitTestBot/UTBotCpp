package org.utbot.cpp.clion.plugin.ui.wizard.steps

import com.intellij.openapi.project.Project
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.bindText
import com.intellij.ui.dsl.builder.columns
import com.intellij.ui.dsl.builder.panel
import org.utbot.cpp.clion.plugin.settings.UTBotSettingsModel
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizardStep
import org.utbot.cpp.clion.plugin.utils.commandLineEditor

class BuildOptionsStep(private val settingsModel: UTBotSettingsModel) : UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/build_dir.html")
        panel {
            row("Relative Path to Build Folder") {
                textField().bindText(settingsModel.projectSettings::buildDirRelativePath).columns(COLUMNS_LARGE)
            }
        }.addToUI()
        addHtml("media/cmake_options.html")
        panel {
            row {
                commandLineEditor({ settingsModel.projectSettings.cmakeOptions },
                    { value: String -> settingsModel.projectSettings.cmakeOptions = value })
            }
        }.addToUI()
    }
}
