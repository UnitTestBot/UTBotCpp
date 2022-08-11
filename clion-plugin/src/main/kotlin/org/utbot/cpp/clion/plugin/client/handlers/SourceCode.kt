package org.utbot.cpp.clion.plugin.client.handlers

import com.intellij.openapi.project.Project
import org.utbot.cpp.clion.plugin.utils.convertFromRemotePathIfNeeded
import testsgen.Util
import java.nio.file.Path

class SourceCode private constructor(
    val localPath: Path,
    val remotePath: String,
    val content: String,
    val regressionMethodsNumber: Int,
    val errorMethodsNumber: Int
) {
    constructor(serverSourceCode: Util.SourceCode, project: Project) : this(
        serverSourceCode.filePath.convertFromRemotePathIfNeeded(project),
        serverSourceCode.filePath,
        serverSourceCode.code,
        serverSourceCode.regressionMethodsNumber,
        serverSourceCode.errorMethodsNumber
    )
}
