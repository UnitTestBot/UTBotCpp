package org.utbot.cpp.clion.plugin.ui.wizard.steps

import org.utbot.cpp.clion.plugin.ui.wizard.UTBotWizardStep


class IntroStrep : UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/intro.html")
    }
}
