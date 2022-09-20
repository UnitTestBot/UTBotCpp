@file:Suppress("UnstableApiUsage")

package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.Disposable
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.ui.ComponentValidator
import com.intellij.openapi.ui.ValidationInfo
import com.intellij.ui.RawCommandLineEditor
import com.intellij.ui.components.JBTextField
import com.intellij.ui.dsl.builder.Cell
import com.intellij.ui.dsl.builder.Row
import com.intellij.ui.layout.PropertyBinding
import javax.swing.JComponent
import javax.swing.JTextField

fun invokeOnEdt(task: () -> Unit) {
    ApplicationManager.getApplication().invokeLater(task)
}

data class ValidationCondition(val errorText: String, val isValid: (JTextField) -> Boolean)
data class ComponentValidationInfo(val component: JComponent, val isValid: () -> Boolean)

fun <T : JBTextField> Cell<T>.addValidation(vararg conditions: ValidationCondition): ComponentValidationInfo {
    this.validationOnInput { textField ->
        conditions.forEach { condition ->
            if (!condition.isValid(textField)) {
                return@validationOnInput ValidationInfo(condition.errorText)
            }
        }
        return@validationOnInput null
    }
    return ComponentValidationInfo(this.component) {
        conditions.all { it.isValid(this.component) }
    }
}

fun JBTextField.validateInput(
    parentDisposable: Disposable,
    vararg conditions: ValidationCondition
): ComponentValidationInfo {
    val validator: () -> ValidationInfo? = lmb@{
        for (condition in conditions) {
            if (!condition.isValid(this)) {
                return@lmb ValidationInfo(condition.errorText, this)
            }
        }
        null
    }

    ComponentValidator(parentDisposable)
        .withValidator(validator)
        .installOn(this)
        .andRegisterOnDocumentListener(this)

    return ComponentValidationInfo(this) { conditions.all { it.isValid(this) } }
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
