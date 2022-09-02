package org.utbot.cpp.clion.plugin.ui

import kotlin.properties.Delegates

// allows attaching multiple listeners for value change
class ObservableValue<T>(initialValue: T) {
    private val changeListeners: MutableList<(T) -> Unit> = mutableListOf()
    var value: T by Delegates.observable(initialValue) { _, oldVal, newVal ->
        changeListeners.forEach {callback ->
            if (oldVal != newVal)
                callback(newVal)
        }
    }

    fun addOnChangeListener(listener: (T) -> Unit) {
        changeListeners.add(listener)
    }
}
