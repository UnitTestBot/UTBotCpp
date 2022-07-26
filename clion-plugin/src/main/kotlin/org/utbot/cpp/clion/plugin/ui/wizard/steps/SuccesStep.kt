package org.utbot.cpp.clion.plugin.ui.wizard.steps

import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizardStep

class SuccessStep : UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/success.html")
    }
}
