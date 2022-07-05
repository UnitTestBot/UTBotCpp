package com.huawei.utbot.cpp.utils

import com.intellij.ui.CollectionListModel

fun <T> CollectionListModel<T>.removeIndices(indices: IntArray) {
    indices.sort()
    val newList = mutableListOf<T>()
    val oldList = toList()
    var i = 0
    for (j in oldList.indices) {
        if (i < indices.size && indices[i] == j) {
            i++
            continue
        }
        newList.add(oldList[j])
    }
    replaceAll(newList)
}
