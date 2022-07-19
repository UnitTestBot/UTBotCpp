package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.psi.PsiDirectory
import org.utbot.cpp.clion.plugin.utils.addDirectoriesRecursive
import org.utbot.cpp.clion.plugin.utils.removeDirectoriesRecursive

interface DirectoriesStatusUpdater {
    fun toggle()
    fun markAsSource()
    fun unmarkAsSource()
}

abstract class BaseUpdater(val selectedDirectories: List<PsiDirectory>) : DirectoriesStatusUpdater {
    abstract fun getCurrentMarkedDirs(): Set<String>
    abstract fun setCurrentMarkedDirs(value: Set<String>)
    override fun markAsSource() {
        setCurrentMarkedDirs(getCurrentMarkedDirs().addDirectoriesRecursive(selectedDirectories))
    }

    override fun unmarkAsSource() {
        setCurrentMarkedDirs(getCurrentMarkedDirs().removeDirectoriesRecursive(selectedDirectories))
    }

    override fun toggle() {
        val currentlyMarked = getCurrentMarkedDirs()
        selectedDirectories.partition {
            it.virtualFile.path in currentlyMarked
        }.also {
            setCurrentMarkedDirs(currentlyMarked.removeDirectoriesRecursive(it.first).addDirectoriesRecursive(it.second))
        }
    }
}
