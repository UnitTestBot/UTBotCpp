package org.utbot.cpp.clion.plugin.ui.wizard.steps

import org.utbot.cpp.clion.plugin.ui.wizard.UTBotBaseWizardStep

class FinalStep : UTBotBaseWizardStep() {
    override fun createUI() = addHtml("media/final_wizard_text.html")
}
