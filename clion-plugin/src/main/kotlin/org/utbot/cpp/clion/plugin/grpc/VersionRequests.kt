package org.utbot.cpp.clion.plugin.grpc

import testsgen.Testgen

//TODO: hardcoding the version is a bad practice, determine it somehow
fun getVersionRequests(): Testgen.VersionInfo = Testgen.VersionInfo.newBuilder().setVersion("2022.7").build()