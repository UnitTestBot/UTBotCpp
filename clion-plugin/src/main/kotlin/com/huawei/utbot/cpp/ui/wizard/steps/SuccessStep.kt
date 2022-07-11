package com.huawei.utbot.cpp.ui.wizard.steps

import com.huawei.utbot.cpp.ui.wizard.UTBotWizardStep

class SuccessStep : UTBotWizardStep() {
    override fun createUI() {
        addHtml("media/success.html")
    }
}
