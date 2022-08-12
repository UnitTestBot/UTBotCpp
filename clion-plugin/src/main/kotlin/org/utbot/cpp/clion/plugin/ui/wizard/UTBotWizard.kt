package org.utbot.cpp.clion.plugin.ui.wizard

import com.intellij.ide.BrowserUtil
import com.intellij.ide.wizard.AbstractWizard
import com.intellij.openapi.project.Project
import javax.swing.JButton
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.settings.UTBotSettingsModel
import org.utbot.cpp.clion.plugin.settings.projectIndependentSettings
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.ui.wizard.steps.BuildOptionsStep
import org.utbot.cpp.clion.plugin.ui.wizard.steps.ConnectionStep
import org.utbot.cpp.clion.plugin.ui.wizard.steps.IntroStep
import org.utbot.cpp.clion.plugin.ui.wizard.steps.FinalStep

class UTBotWizard(private val project: Project) : AbstractWizard<UTBotBaseWizardStep>("UTBot: Quickstart", project) {
    // copy of settings to make changes during wizard steps
    private val mySettingsModel = UTBotSettingsModel(project.settings.storedSettings, projectIndependentSettings)

    init {
        addStep(IntroStep())
        addStep(ConnectionStep(project, mySettingsModel))
        addStep(BuildOptionsStep(mySettingsModel))
        addStep(FinalStep())
        super.init()
        isResizable = true
        setSize(400, 400)
        getButton(myOKAction)?.text = "Finish"
    }

    /**
     * Commits settings changes made in wizard and sends a notification.
     */
    override fun doOKAction() {
        project.settings.storedSettings.fromSettingsModel(mySettingsModel)
        projectIndependentSettings.fromSettingsModel(mySettingsModel)
        projectIndependentSettings.fireConnectionSettingsChanged()

        with(project.settings) {
            fireUTBotSettingsChanged()
        }

        super.doOKAction()
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
