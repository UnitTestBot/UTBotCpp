package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.Disposable
import com.intellij.openapi.ui.ComponentValidator
import com.intellij.openapi.ui.ValidationInfo
import com.intellij.ui.DocumentAdapter
import com.intellij.ui.RawCommandLineEditor
import com.intellij.ui.dsl.builder.Row
import com.intellij.ui.layout.PropertyBinding
import javax.swing.JTextField
import javax.swing.event.DocumentEvent

fun JTextField.validateOnInput(parentDisposable: Disposable, validator: ()->ValidationInfo?) {
    ComponentValidator(parentDisposable).withValidator(validator).installOn(this)
    document.addDocumentListener(
        object : DocumentAdapter() {
            override fun textChanged(p0: DocumentEvent) {
                ComponentValidator.getInstance(this@validateOnInput).ifPresent { v ->
                    v.revalidate()
                }
            }
        }
    )
}

fun Row.commandLineEditor(getter: () -> String, setter: (String) -> Unit, width: Int = 600) {
    cell(RawCommandLineEditor()).bind({ comp -> comp.text }, { comp, value: String -> comp.text = value },
        PropertyBinding(
            { getter() },
            { value: String -> setter(value) })
    ).applyToComponent {
        preferredSize = preferredSize.also {
            it.width = width
        }
    }
}
