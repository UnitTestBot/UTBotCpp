package com.huawei.utbot.cpp.ui.sourceFoldersView

import com.huawei.utbot.cpp.utils.markDirectoriesRecursive
import com.huawei.utbot.cpp.utils.unmarkDirectoriesRecursive
import com.intellij.psi.PsiDirectory

interface DirectoriesStatusUpdater {
    fun toggle()
    fun markAsSource()
    fun unmarkAsSource()
}

abstract class BaseUpdater(val selectedDirectories: List<PsiDirectory>) : DirectoriesStatusUpdater {
    abstract fun getCurrentMarkedDirs(): Set<String>
    abstract fun setCurrentMarkedDirs(value: Set<String>)
    override fun markAsSource() {
        setCurrentMarkedDirs(getCurrentMarkedDirs().markDirectoriesRecursive(selectedDirectories))
    }

    override fun unmarkAsSource() {
        setCurrentMarkedDirs(getCurrentMarkedDirs().unmarkDirectoriesRecursive(selectedDirectories))
    }

    override fun toggle() {
        val currentlyMarked = getCurrentMarkedDirs()
        selectedDirectories.partition {
            it.virtualFile.path in currentlyMarked
        }.also {
            setCurrentMarkedDirs(currentlyMarked.unmarkDirectoriesRecursive(it.first).markDirectoriesRecursive(it.second))
        }
    }
}
