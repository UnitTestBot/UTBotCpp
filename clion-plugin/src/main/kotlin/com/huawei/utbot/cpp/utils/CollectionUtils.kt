package com.huawei.utbot.cpp.utils

import com.intellij.ui.CollectionListModel

fun <T> CollectionListModel<T>.removeIndices(indices: IntArray) {
    val sortedIndices = indices.sorted()
    with(toList()) {
        removeAll(
            filterIndexed { idx, _ ->
                sortedIndices.binarySearch(idx) >= 0
            }
        )
        this@removeIndices.replaceAll(this)
    }
}
