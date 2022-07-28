package org.utbot.cpp.clion.plugin.actions.generate

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.ui.ComponentValidator
import com.intellij.openapi.ui.ValidationInfo
import com.intellij.openapi.ui.popup.JBPopup
import com.intellij.openapi.ui.popup.JBPopupFactory
import com.intellij.ui.DocumentAdapter
import com.intellij.ui.components.fields.ExtendableTextField
import javax.swing.ListSelectionModel
import javax.swing.event.DocumentEvent
import org.utbot.cpp.clion.plugin.grpc.getFunctionGrpcRequest
import org.utbot.cpp.clion.plugin.grpc.getPredicateGrpcRequest
import org.utbot.cpp.clion.plugin.client.requests.FunctionReturnTypeRequest
import org.utbot.cpp.clion.plugin.client.requests.PredicateRequest
import org.utbot.cpp.clion.plugin.utils.activeProject
import org.utbot.cpp.clion.plugin.utils.currentClient
import org.utbot.cpp.clion.plugin.utils.notifyError
import testsgen.Util.ValidationType
import java.awt.Dimension
import java.awt.event.KeyAdapter
import java.awt.event.KeyEvent
import java.math.BigInteger
import java.util.function.Supplier

/**
 * Action to take needed information from user and trigger generation for predicate.
 *
 * Predicate request generates tests that satisfy some condition (predicate) for
 * function return type. E.g. tests where return value (function return type is int)
 * is smaller than 10.
 *
 * To assemble predicate request we need to know function return type.
 * For that we send request to server @see [FunctionReturnTypeRequest].
 * Depending on return type we ask user what comparison operator to use.
 * For example, if return type is bool, we suggest == or !=, if it is
 * not bool, then available operators are: ==, <=, >=, <, >. @see [chooseComparisonOperator]
 * Then we ask user for a value to compare with, @see [chooseReturnValue].
 * Then we assemble the predicate request and launch its execution @see [sendPredicateToServer].
 *
 * For asking comparison operator and return value we use popups.
 */
class GenerateForPredicateAction : BaseGenerateTestsAction() {
    override fun actionPerformed(e: AnActionEvent) {
        // when we gathered all needed information for predicate request, assemble it and execute it.
        fun sendPredicateToServer(validationType: ValidationType, valueToCompare: String, comparisonOperator: String) =
            PredicateRequest(
                getPredicateGrpcRequest(e, comparisonOperator, validationType, valueToCompare),
                e.activeProject()
            ).apply {
                e.currentClient.executeRequestIfNotDisposed(this)
            }

        // ask for comparison operator to use in predicate
        fun chooseComparisonOperator(
            validationType: ValidationType,
            proceedWithComparisonOperator: (comparisonOperator: String) -> Unit,
        ) {
            when (validationType) {
                ValidationType.STRING,
                ValidationType.BOOL -> {
                    proceedWithComparisonOperator("==")
                    return
                }
                ValidationType.UNSUPPORTED -> {
                    notifyError(
                        "Unsupported return type for \'Generate Tests With Prompted Result\' feature: \n" +
                                "supported types are integers, booleans, characters, floats and strings"
                    )
                    return
                }
                ValidationType.UNRECOGNIZED -> {
                    notifyError(
                        "Could not recognise return type for 'Generate Tests With Prompted Result' feature: \n" +
                                "supported types are integers, booleans, characters, floats and strings"
                    )
                    return
                }
                else -> {
                    val operators = listOf("==", "<=", "=>", "<", ">")
                    createListPopup("Select predicate", operators) { comparisonOperator ->
                        proceedWithComparisonOperator(comparisonOperator)
                    }.showInBestPositionFor(e.dataContext)
                }
            }
        }

        // ask for return value of the function to compare with
        fun chooseReturnValue(
            validationType: ValidationType,
            proceedWithValueToCompare: (valueToCompare: String) -> Unit,
        ) {
            val popup = when (validationType) {
                ValidationType.BOOL -> createTrueFalsePopup { returnValue -> proceedWithValueToCompare(returnValue) }
                else -> createTextFieldPopup(validationType) { returnValue -> proceedWithValueToCompare(returnValue) }
            }
            popup.showInBestPositionFor(e.dataContext)
        }
        //ask server for return type
        FunctionReturnTypeRequest(
            getFunctionGrpcRequest(e),
            e.activeProject(),
        ) { functionReturnType ->
            val validationType = functionReturnType.validationType
            // then ask for comparison operator to use from user
            chooseComparisonOperator(validationType) { comparisonOperator ->
                // then ask for return value to compare with from user
                chooseReturnValue(validationType) { valueToCompare ->
                    // when we have all needed information, assemble and execute
                    sendPredicateToServer(validationType, valueToCompare, comparisonOperator)
                }
            }
        }.apply {
            e.currentClient.executeRequestIfNotDisposed(this)
        }
    }

    override fun isDefined(e: AnActionEvent): Boolean = e.project != null

    private fun createListPopup(title: String, items: List<String>, onChoose: (String) -> Unit): JBPopup {
        return JBPopupFactory.getInstance().createPopupChooserBuilder(items)
            .setResizable(false)
            .setMovable(false)
            .setTitle(title)
            .setSelectionMode(ListSelectionModel.SINGLE_SELECTION)
            .setCloseOnEnter(true)
            .setItemChosenCallback(onChoose)
            .createPopup()
    }

    private fun createTrueFalsePopup(onChoose: (String) -> Unit) =
        createListPopup("Select bool value", listOf("true", "false")) { onChoose(it) }

    //creates popup with input textfield, used for asking return value
    private fun createTextFieldPopup(type: ValidationType, onChoose: (String) -> Unit): JBPopup {
        val textField = ExtendableTextField()
        textField.minimumSize = Dimension(100, textField.width)
        textField.text = defaultReturnValues[type]
        textField.selectAll()

        val popup = JBPopupFactory.getInstance().createComponentPopupBuilder(textField, null)
            .setFocusable(true)
            .setRequestFocus(true)
            .setTitle("Specify Return Value of type ${validationTypeName[type]}")
            .createPopup()

        var canClosePopup = true
        ComponentValidator(popup).withValidator(Supplier<ValidationInfo?> {
            val validationResult = returnValueValidators[type]?.let { it(textField.text) }
            if (validationResult is ValidationResult.Failure) {
                canClosePopup = false
                ValidationInfo(validationResult.message, textField)
            } else {
                canClosePopup = true
                null
            }
        }).installOn(textField)

        textField.document.addDocumentListener(object : DocumentAdapter() {
            override fun textChanged(p0: DocumentEvent) {
                ComponentValidator.getInstance(textField).ifPresent { v ->
                    v.revalidate()
                }
            }
        })

        textField.addKeyListener(object : KeyAdapter() {
            override fun keyPressed(e: KeyEvent) {
                if (e.keyCode == KeyEvent.VK_ENTER) {
                    if (canClosePopup) {
                        popup.cancel()
                        onChoose(textField.text)
                    }
                }
            }
        })

        return popup
    }

    companion object {
        //Note: server does not support some types like byte or boolean
        val defaultReturnValues = mapOf(
            ValidationType.INT8_T to "0",
            ValidationType.INT16_T to "0",
            ValidationType.INT32_T to "0",
            ValidationType.INT64_T to "0",
            ValidationType.UINT8_T to "0",
            ValidationType.UINT16_T to "0",
            ValidationType.UINT32_T to "0",
            ValidationType.UINT64_T to "0",
            ValidationType.CHAR to "a",
            ValidationType.FLOAT to "1.0",
            ValidationType.STRING to "default",
        )

        private fun isIntegerInBounds(value: String, low: BigInteger?, high: BigInteger?): Boolean {
            if (low == null || high == null) {
                return false
            }
            return value.toBigInteger() in low..high
        }

        private fun intBoundsBySize(size: Int, signed: Boolean): Pair<BigInteger, BigInteger> {
            if (!signed) {
                return Pair((0).toBigInteger(), (2).toBigInteger().pow(size).dec())
            }
            return Pair((2).toBigInteger().pow(size - 1).unaryMinus(), (2).toBigInteger().pow(size - 1).dec())
        }

        private val validationTypeName = mapOf(
            ValidationType.INT8_T to "int8_t",
            ValidationType.INT16_T to "int16_t",
            ValidationType.INT32_T to "int32_t",
            ValidationType.INT64_T to "int64_t",
            ValidationType.UINT8_T to "uint8_t",
            ValidationType.UINT16_T to "uint16_t",
            ValidationType.UINT32_T to "uint32_t",
            ValidationType.UINT64_T to "uint64_t",
            ValidationType.CHAR to "char",
            ValidationType.FLOAT to "float",
            ValidationType.STRING to "string",
            ValidationType.BOOL to "bool"
        )

        private val integerBounds = mapOf(
            ValidationType.INT8_T to intBoundsBySize(8, false),
            ValidationType.INT16_T to intBoundsBySize(16, false),
            ValidationType.INT32_T to intBoundsBySize(32, false),
            ValidationType.INT64_T to intBoundsBySize(64, false),
            ValidationType.UINT8_T to intBoundsBySize(8, true),
            ValidationType.UINT16_T to intBoundsBySize(16, true),
            ValidationType.UINT32_T to intBoundsBySize(32, true),
            ValidationType.UINT64_T to intBoundsBySize(64, true)
        )

        sealed class ValidationResult {
            object Success : ValidationResult()
            data class Failure(val message: String) : ValidationResult()
        }

        private fun intValidationFunc(validationType: ValidationType): (String) -> ValidationResult =
            fun(value: String): ValidationResult = if ("""^-?(([1-9][0-9]*)|0)$""".toRegex().matches(value)) {
                if (isIntegerInBounds(
                        value,
                        integerBounds[validationType]?.first,
                        integerBounds[validationType]?.second
                    )
                ) {
                    ValidationResult.Success
                } else {
                    ValidationResult.Failure("Value does not fit into C  ${validationTypeName[validationType]} type")
                }
            } else {
                ValidationResult.Failure("Value is not an integer")
            }

        private fun validateChar(value: String): ValidationResult {
            val escapeSequence = listOf("\\\'", "\"", "\\?", "\\\\", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v")
            return if (value.length == 1 || escapeSequence.contains(value)) {
                ValidationResult.Success
            } else {
                ValidationResult.Failure("Value is not a character")
            }
        }

        private fun validateFloat(value: String): ValidationResult {
            return if ("""^-?([1-9][0-9]*)[.]([0-9]*)$""".toRegex().matches(value)) {
                if (value.length < 15) {
                    ValidationResult.Success
                } else {
                    ValidationResult.Failure("Value does not fit into C float type")
                }
            } else {
                ValidationResult.Failure("Value is not floating-point")
            }
        }

        private fun validateString(value: String): ValidationResult =
            if (value.length > 32) {
                ValidationResult.Failure("String is too long")
            } else {
                ValidationResult.Success
            }

        val returnValueValidators = mapOf(
            ValidationType.INT8_T to intValidationFunc(ValidationType.INT8_T),
            ValidationType.INT16_T to intValidationFunc(ValidationType.INT16_T),
            ValidationType.INT32_T to intValidationFunc(ValidationType.INT32_T),
            ValidationType.INT64_T to intValidationFunc(ValidationType.INT64_T),
            ValidationType.UINT8_T to intValidationFunc(ValidationType.UINT8_T),
            ValidationType.UINT16_T to intValidationFunc(ValidationType.UINT16_T),
            ValidationType.UINT32_T to intValidationFunc(ValidationType.UINT32_T),
            ValidationType.UINT64_T to intValidationFunc(ValidationType.UINT64_T),
            ValidationType.CHAR to this::validateChar,
            ValidationType.FLOAT to this::validateFloat,
            ValidationType.STRING to this::validateString,
        )
    }
}
