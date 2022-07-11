package com.huawei.utbot.cpp.ui.wizard.steps

import com.huawei.utbot.cpp.ui.wizard.UTBotWizardStep

class IntroStrep : UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/intro.html")
    }
}
