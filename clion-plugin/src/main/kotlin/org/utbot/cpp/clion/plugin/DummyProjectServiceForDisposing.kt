package org.utbot.cpp.clion.plugin

import com.intellij.openapi.Disposable
import com.intellij.openapi.components.Service
import com.intellij.openapi.project.Project


/**
 * A project level service to be used as parent disposable for disposables
 * with a project lifetime
 */
@Service
class DummyProjectServiceForDisposing(val project: Project): Disposable {
    override fun dispose() {}
}
