package org.utbot.cpp.clion.plugin.ui

import kotlin.properties.Delegates

// allows attaching multiple listeners for value change
class ObservableValue<T>(initialValue: T) {
    private val changeListeners: MutableList<(T) -> Unit> = mutableListOf()
    var value: T by Delegates.observable(initialValue) { _, _, newVal ->
        changeListeners.forEach {
            it(newVal)
        }
    }

    fun addOnChangeListener(listener: (T) -> Unit) {
        changeListeners.add(listener)
    }
}
