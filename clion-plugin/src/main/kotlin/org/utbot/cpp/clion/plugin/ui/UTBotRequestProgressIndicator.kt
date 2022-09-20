package org.utbot.cpp.clion.plugin.ui

import com.intellij.openapi.progress.TaskInfo
import com.intellij.openapi.progress.util.AbstractProgressIndicatorExBase
import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.ex.StatusBarEx
import com.intellij.openapi.wm.ex.WindowManagerEx
import kotlinx.coroutines.Job
import org.utbot.cpp.clion.plugin.UTBot
import org.utbot.cpp.clion.plugin.utils.invokeOnEdt
import org.utbot.cpp.clion.plugin.utils.notifyInfo

class UTBotRequestProgressIndicator(
    private val taskDisplayName: String,
    private val requestJob: Job? = null,
    private val project: Project,
) : AbstractProgressIndicatorExBase(true) {

    private val requestTask = UTBotRequestTaskInfo(taskDisplayName)

    init {
        isIndeterminate = false
    }

    override fun start() {
        val frame = WindowManagerEx.getInstanceEx().findFrameFor(project) ?: return
        val statusBar = frame.statusBar as? StatusBarEx ?: return
        invokeOnEdt {
            statusBar.addProgress(this, requestTask)
        }
        super.start()
    }

    override fun stop() {
        requestJob?.cancel()
        finish()
        super.stop()
    }

    fun finish() = finish(requestTask)

    override fun cancel() {
        requestJob?.cancel()
        finish(requestTask)
        super.cancel()
        notifyInfo(
            UTBot.message("notify.cancelled.request.title"),
            UTBot.message("notify.cancelled.request", taskDisplayName)
        )
    }

    private class UTBotRequestTaskInfo(private val titleText: String) : TaskInfo {
        override fun getTitle() = titleText
        override fun getCancelText() = "Cancelling Request"
        override fun getCancelTooltipText() = ""
        override fun isCancellable() = true
    }
}


