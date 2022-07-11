package com.huawei.utbot.cpp.ui.wizard

import com.huawei.utbot.cpp.UTBot
import com.huawei.utbot.cpp.ui.wizard.steps.BuildOptionsStep
import com.huawei.utbot.cpp.ui.wizard.steps.ConnectionStep
import com.huawei.utbot.cpp.ui.wizard.steps.IntroStrep
import com.huawei.utbot.cpp.ui.wizard.steps.SuccessStep
import com.huawei.utbot.cpp.utils.utbotSettings
import com.intellij.ide.BrowserUtil
import com.intellij.ide.wizard.AbstractWizard
import com.intellij.openapi.project.Project

class UTBotWizard(private val project: Project) : AbstractWizard<UTBotWizardStep>("UTBot: Quickstart", project) {
    // copy of settings to make changes during wizard steps
    private val mySettingsModel = project.utbotSettings.state.copy()

    init {
        addStep(IntroStrep())
        addStep(ConnectionStep(project, mySettingsModel, disposable))
        addStep(BuildOptionsStep(project, mySettingsModel))
        addStep(SuccessStep())
        super.init()
        isResizable = true
        setSize(400, 400)
    }

    override fun doOKAction() {
        super.doOKAction()
        // commit changes made during wizard and notify
        with(project.utbotSettings) {
            loadState(mySettingsModel)
            fireUTBotSettingsChanged()
        }
    }

    override fun proceedToNextStep() {
        if (currentStepObject?.canProceedToNextStep() != false) {
            super.proceedToNextStep()
        }
    }

    override fun helpAction() {
        BrowserUtil.browse(UTBot.message("uri.wiki"))
    }

    override fun doHelpAction() {
        helpAction()
    }

    override fun getHelpID(): String? = null
}
