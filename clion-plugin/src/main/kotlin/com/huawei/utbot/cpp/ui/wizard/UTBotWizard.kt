package com.huawei.utbot.cpp.ui.wizard

import com.intellij.ide.BrowserUtil
import com.intellij.ide.wizard.AbstractWizard
import com.intellij.openapi.project.Project

class UTBotWizard(project: Project) : AbstractWizard<UTBotWizardStep>("UTBot: Quickstart", project) {
    init {
        addStep(IntroStrep())
        addStep(ConnectionStep(project))
        addStep(RemotePathStep(project))
        addStep(BuildOptionsStep(project))
        addStep(SuccessStep())
        super.init()
        isResizable = true
        setSize(400, 400)
    }

    override fun helpAction() {
        BrowserUtil.browse("https://github.com/UnitTestBot/UTBotCpp/wiki")
    }

    override fun doHelpAction() {
        helpAction()
    }

    override fun getHelpID(): String? = null
}
