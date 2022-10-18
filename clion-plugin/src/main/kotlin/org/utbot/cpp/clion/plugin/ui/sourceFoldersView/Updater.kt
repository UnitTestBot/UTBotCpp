package org.utbot.cpp.clion.plugin.ui.sourceFoldersView

import com.intellij.psi.PsiDirectory
import org.utbot.cpp.clion.plugin.utils.localPath
import org.utbot.cpp.clion.plugin.utils.visitAllDirectories

interface DirectoriesStatusUpdater {
    fun toggle()
    fun markAsSource()
    fun unmarkAsSource()
}

abstract class BaseUpdater(private val selectedDirectories: List<PsiDirectory>) : DirectoriesStatusUpdater {
    abstract fun getCurrentMarkedDirs(): Set<String>
    abstract fun setCurrentMarkedDirs(value: Set<String>)

    override fun markAsSource() =
        setCurrentMarkedDirs(getCurrentMarkedDirs().addDirectoriesRecursively(selectedDirectories))

    override fun unmarkAsSource() =
        setCurrentMarkedDirs(getCurrentMarkedDirs().removeDirectoriesRecursively(selectedDirectories))

    override fun toggle() {
        val currentlyMarked = getCurrentMarkedDirs()
        selectedDirectories.partition {
            it.virtualFile.localPath.toString() in currentlyMarked
        }.also {
            setCurrentMarkedDirs(currentlyMarked.removeDirectoriesRecursively(it.first).addDirectoriesRecursively(it.second))
        }
    }

    // merge directories' paths from this set, with all the directories from passed list including nested directories
    private fun Set<String>.addDirectoriesRecursively(dirsToAdd: List<PsiDirectory>): Set<String> {
        val newSourceFolders = this.toMutableSet()
        dirsToAdd.forEach { dir ->
            newSourceFolders.add(dir.virtualFile.localPath.toString())
            dir.virtualFile.localPath.visitAllDirectories {
                newSourceFolders.add(it.toString())
            }
        }
        return newSourceFolders
    }

    // subtract from this set of directories the passed directories including nested directories
    private fun Set<String>.removeDirectoriesRecursively(dirsToRemove: List<PsiDirectory>): Set<String> {
        val newSourceFolders = this.toMutableSet()
        dirsToRemove.forEach { dir ->
            newSourceFolders.add(dir.virtualFile.localPath.toString())
            dir.virtualFile.localPath.visitAllDirectories {
                newSourceFolders.remove(it.toString())
            }
        }
        return newSourceFolders
    }
}
