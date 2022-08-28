package org.utbot.cpp.clion.plugin.client.handlers.testsStreamHandler

class CMakePrinter(private val currentCMakeListsContent: String) {
    private val ss = StringBuilder()
    var isEmpty = true
    private set

    init {
        ss.append("\n")
        startUTBotSection()
    }

    private fun add(string: String) {
        ss.append(string)
        ss.append("\n")
    }

    fun startUTBotSection() {
        add("#utbot_section_start")
    }

    fun addDownloadGTestSection() {
        isEmpty = false
        add(
            """
            include(FetchContent)
            FetchContent_Declare(
               googletest
               GIT_REPOSITORY https://github.com/google/googletest.git
               GIT_TAG release-1.12.1
            )
            # For Windows: Prevent overriding the parent project's compiler/linker settings
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(googletest)
            enable_testing()
            include(GoogleTest)
        """.trimIndent()
        )
    }

    fun addSubdirectory(dirRelativePath: String) {
        isEmpty = false
        val addDirectoryInstruction = "add_subdirectory($dirRelativePath)"
        if (!currentCMakeListsContent.contains(addDirectoryInstruction))
            add(addDirectoryInstruction)
    }

    fun get(): String {
        return ss.toString() + "#utbot_section_end\n"
    }
}

