@file:Suppress("UnstableApiUsage")

package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.Disposable
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.ui.ComponentValidator
import com.intellij.openapi.ui.ValidationInfo
import com.intellij.ui.DocumentAdapter
import com.intellij.ui.RawCommandLineEditor
import com.intellij.ui.dsl.builder.Row
import com.intellij.ui.layout.PropertyBinding
import javax.swing.JSpinner
import javax.swing.JTextField
import javax.swing.event.DocumentEvent

fun invokeOnEdt(task: () -> Unit) {
    ApplicationManager.getApplication().invokeLater(task)
}

fun JTextField.validateInput(parentDisposable: Disposable, validator: () -> ValidationInfo?) {
    ComponentValidator(parentDisposable)
        .withValidator(validator)
        .installOn(this)

    document.addDocumentListener(
        object : DocumentAdapter() {
            override fun textChanged(e: DocumentEvent) {
                ComponentValidator
                    .getInstance(this@validateInput)
                    .ifPresent { v -> v.revalidate() }
            }
        }
    )
}

fun Row.commandLineEditor(
    getter: () -> String,
    setter: (String) -> Unit,
) {
    cell(RawCommandLineEditor()).bind(
        { comp -> comp.text },
        { comp, value: String -> comp.text = value },
        PropertyBinding(
            { getter() },
            { value: String -> setter(value) }
        )
    ).applyToComponent {
        preferredSize = preferredSize.also { it.width = 600 }
    }
}
