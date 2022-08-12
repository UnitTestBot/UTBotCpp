package org.utbot.cpp.clion.plugin.ui.wizard.steps

import org.utbot.cpp.clion.plugin.ui.wizard.UTBotBaseWizardStep

class IntroStep : UTBotBaseWizardStep() {
    override fun createUI() = addHtml("media/intro_wizard_text.html")
}
