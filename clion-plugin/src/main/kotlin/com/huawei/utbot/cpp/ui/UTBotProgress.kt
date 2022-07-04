package com.huawei.utbot.cpp.ui

import com.huawei.utbot.cpp.utils.invokeOnEdt
import com.huawei.utbot.cpp.utils.notifyInfo
import com.intellij.openapi.progress.TaskInfo
import com.intellij.openapi.progress.util.AbstractProgressIndicatorExBase
import com.intellij.openapi.project.Project
import com.intellij.openapi.wm.ex.StatusBarEx
import com.intellij.openapi.wm.ex.WindowManagerEx
import kotlinx.coroutines.Job

class UTBotRequestProgressIndicator(val name: String, var requestJob: Job? = null, val project: Project) : AbstractProgressIndicatorExBase(true) {
    val task = UTBotRequestTaskInfo(name)

    init {
        isIndeterminate = false
    }

    override fun start() {
        val frame = WindowManagerEx.getInstanceEx().findFrameFor(project) ?: return
        val statusBar = frame.statusBar as? StatusBarEx ?: return
        invokeOnEdt {
            statusBar.addProgress(this, task)
        }
        super.start()
    }

    override fun stop() {
        requestJob?.cancel()
        super.stop()
    }

    fun complete() {
        finish(task)
    }

    override fun cancel() {
        requestJob?.cancel()
        notifyInfo("Successfully canceled: $name")
        finish(task)
        super.cancel()
    }
}

class UTBotRequestTaskInfo(val titleText: String) : TaskInfo {
    override fun getTitle() = titleText

    override fun getCancelText() = "Cancelling Request"

    override fun getCancelTooltipText() = ""

    override fun isCancellable() = true
}
