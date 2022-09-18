package org.utbot.cpp.clion.plugin.client.requests.test

import com.intellij.openapi.project.Project
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.Flow
import org.utbot.cpp.clion.plugin.actions.FocusAction
import org.utbot.cpp.clion.plugin.client.handlers.TestsStreamHandler
import org.utbot.cpp.clion.plugin.client.requests.BaseRequest
import org.utbot.cpp.clion.plugin.grpc.Params
import org.utbot.cpp.clion.plugin.grpc.RemoteMapping
import org.utbot.cpp.clion.plugin.settings.settings
import org.utbot.cpp.clion.plugin.utils.getLongestCommonPathFromRoot
import org.utbot.cpp.clion.plugin.utils.isHeaderFile
import org.utbot.cpp.clion.plugin.utils.isSarifReport
import org.utbot.cpp.clion.plugin.utils.logger
import org.utbot.cpp.clion.plugin.utils.notifyInfo
import org.utbot.cpp.clion.plugin.utils.path
import testsgen.Testgen
import java.nio.file.Path

/**
 * Base class for requests that handle a stream of [Testgen.TestsResponse].
 * @param progressName - a name of a progress that user will see, when this request will be executing.
 */
abstract class BaseTestsRequest<R>(params: Params<R>, project: Project, private val progressName: String) :
    BaseRequest<R, Flow<Testgen.TestsResponse>>(params, project) {
    val logger = project.logger
    override fun build(): R {
        val mapping = RemoteMapping(project.path, project.settings.storedSettings.remotePath)
        return params.build(mapping)
    }

    override suspend fun Flow<Testgen.TestsResponse>.handle(cancellationJob: Job?) {
        if (cancellationJob?.isActive == true) {
            TestsStreamHandler(
                project,
                this,
                progressName,
                cancellationJob,
                ::notifySuccess,
                ::notifyError
            ).handle()
        }
    }

    open fun getFocusTarget(generatedTestFiles: List<Path>): Path? =
        generatedTestFiles
            .filter { !isHeaderFile(it) && !it.isSarifReport() }
            .getLongestCommonPathFromRoot()

    open fun getInfoMessage() = "Tests generated!"

    open fun notifySuccess(generatedTestFiles: List<Path>) {
        notifyInfo(getInfoMessage(), project, getFocusTarget(generatedTestFiles)?.let {
            FocusAction(it)
        })
    }

    open fun notifyError(cause: Throwable) {}
}