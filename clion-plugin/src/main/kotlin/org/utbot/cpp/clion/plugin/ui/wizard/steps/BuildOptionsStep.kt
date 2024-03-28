@file:Suppress("UnstableApiUsage")

package org.utbot.cpp.clion.plugin.ui.wizard.steps

import com.intellij.openapi.Disposable
import com.intellij.ui.dsl.builder.COLUMNS_LARGE
import com.intellij.ui.dsl.builder.bindText
import com.intellij.ui.dsl.builder.columns
import com.intellij.ui.dsl.builder.panel
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.settings.UTBotSettingsModel
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotBaseWizardStep
import org.utbot.cpp.clion.plugin.utils.ValidationCondition
import org.utbot.cpp.clion.plugin.utils.commandLineEditor

class BuildOptionsStep(parentDisposable: Disposable, private val settingsModel: UTBotSettingsModel) : UTBotBaseWizardStep(parentDisposable) {
    override fun createUI() {
        addHtml("media/options_wizard_text.html")
        panel {
            row("Relative path to Build directory") {
                textField().bindText(settingsModel.projectSettings::buildDirRelPath).columns(COLUMNS_LARGE)
                    .validateWith(ValidationCondition(UTBot.message("validation.not.empty")) { it.text.isNotEmpty() })
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
