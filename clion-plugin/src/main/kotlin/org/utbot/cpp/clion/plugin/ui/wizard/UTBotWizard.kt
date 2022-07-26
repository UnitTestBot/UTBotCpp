package org.utbot.cpp.clion.plugin.ui.wizard

import com.intellij.ide.BrowserUtil
import com.intellij.ide.wizard.AbstractWizard
import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.ui.wizard.steps.BuildOptionsStep
import org.utbot.cpp.clion.plugin.ui.wizard.steps.ConnectionStep
import org.utbot.cpp.clion.plugin.ui.wizard.steps.IntroStrep
import org.utbot.cpp.clion.plugin.ui.wizard.steps.SuccessStep
import org.utbot.cpp.clion.plugin.utils.utbotSettings

class UTBotWizard(private val project: Project) : AbstractWizard<UTBotWizardStep>("UTBot: Quickstart", project) {
    // copy of settings to make changes during wizard steps
    private val mySettingsModel = project.utbotSettings.asModel()

    init {
        addStep(IntroStrep())
        addStep(ConnectionStep(project, mySettingsModel, disposable))
        addStep(BuildOptionsStep(mySettingsModel))
        addStep(SuccessStep())
        super.init()
        isResizable = true
        setSize(400, 400)
    }

    override fun doOKAction() {
        super.doOKAction()
        // commit changes made during wizard and notify
        with(project.utbotSettings) {
            project.utbotSettings.applyModel(mySettingsModel)
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
