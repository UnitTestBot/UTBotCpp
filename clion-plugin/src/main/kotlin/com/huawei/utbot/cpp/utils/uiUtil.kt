package com.huawei.utbot.cpp.utils

import com.intellij.openapi.Disposable
import com.intellij.openapi.ui.ComponentValidator
import com.intellij.openapi.ui.ValidationInfo
import com.intellij.ui.DocumentAdapter
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
