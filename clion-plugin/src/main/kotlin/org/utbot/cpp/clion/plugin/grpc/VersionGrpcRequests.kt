package org.utbot.cpp.clion.plugin.grpc

import testsgen.Testgen

//TODO: when plugin is ready for release, take version from publish github action.
fun getVersionGrpcRequest(): Testgen.VersionInfo = Testgen.VersionInfo.newBuilder().setVersion("0.0.1").build()
