package org.utbot.cpp.clion.plugin.ui.wizard.steps

import com.intellij.openapi.Disposable
import org.utbot.cpp.clion.plugin.ui.wizard.UTBotBaseWizardStep

class IntroStep(parentDisposable: Disposable) : UTBotBaseWizardStep(parentDisposable) {
    override fun createUI() = addHtml("media/intro_wizard_text.html")
}
