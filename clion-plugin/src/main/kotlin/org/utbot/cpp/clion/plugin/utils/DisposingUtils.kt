package org.utbot.cpp.clion.plugin.utils

import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.components.service
import com.intellij.openapi.project.Project

/**
 * A project level service to be used as parent disposable
 * for disposables (objects that need to do some cleanup) with project lifetime
 *
 * Must not be used as a child disposable, because it is a child for [ClientManager] disposable
 */
@Service
class ProjectLifetimeDisposable(project: Project) : Disposable {
    override fun dispose() {}
}

val Project.projectLifetimeDisposable: ProjectLifetimeDisposable get() = this.service()
