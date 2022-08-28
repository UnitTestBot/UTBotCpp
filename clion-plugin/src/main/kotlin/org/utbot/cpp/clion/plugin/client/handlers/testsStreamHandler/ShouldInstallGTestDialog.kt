package org.utbot.cpp.clion.plugin.client.handlers.testsStreamHandler

import com.intellij.openapi.project.Project
import com.intellij.openapi.ui.DialogWrapper
import com.intellij.ui.dsl.builder.panel
import javax.swing.JComponent
import org.utbot.cpp.clion.plugin.UTBot

class ShouldInstallGTestDialog(project: Project) : DialogWrapper(project) {
    init {
        init()
        title = "UTBot: GTest Install"
    }

    override fun createCenterPanel(): JComponent {
        return panel {
            row(UTBot.message("dialog.should.install.gtest")) {}
        }
    }
}
