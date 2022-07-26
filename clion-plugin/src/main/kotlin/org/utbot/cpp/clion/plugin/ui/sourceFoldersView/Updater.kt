package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.psi.PsiDirectory
import org.utbot.cpp.clion.plugin.utils.visitAllDirectories

interface DirectoriesStatusUpdater {
    fun toggle()
    fun markAsSource()
    fun unmarkAsSource()
}

abstract class BaseUpdater(val selectedDirectories: List<PsiDirectory>) : DirectoriesStatusUpdater {
    // merge directories' paths from this set, with all the directories from passed list including nested directories
    private fun Set<String>.addDirectoriesRecursive(dirsToAdd: List<PsiDirectory>): Set<String> {
        val newSourceFolders = this.toMutableSet()
        dirsToAdd.forEach { dir ->
            newSourceFolders.add(dir.virtualFile.path)
            dir.virtualFile.toNioPath().visitAllDirectories {
                newSourceFolders.add(it.toString())
            }
        }
        return newSourceFolders
    }

    // subtract from this set of directories the passed directories including nested directories
    private fun Set<String>.removeDirectoriesRecursive(dirsToRemove: List<PsiDirectory>): Set<String> {
        val newSourceFolders = this.toMutableSet()
        dirsToRemove.forEach { dir ->
            newSourceFolders.add(dir.virtualFile.path)
            dir.virtualFile.toNioPath().visitAllDirectories {
                newSourceFolders.remove(it.toString())
            }
        }
        return newSourceFolders
    }

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
