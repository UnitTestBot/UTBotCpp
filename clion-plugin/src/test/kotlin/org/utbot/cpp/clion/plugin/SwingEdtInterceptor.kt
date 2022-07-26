package org.utbot.cpp.clion.plugin

import com.intellij.testFramework.runInEdtAndWait
import org.junit.jupiter.api.extension.ExtensionContext
import org.junit.jupiter.api.extension.InvocationInterceptor
import org.junit.jupiter.api.extension.ReflectiveInvocationContext
import java.lang.reflect.Method

// we are allowed to access UI only from EDT, so this class
// is used to run every test method in EDT
class SwingEdtInterceptor : InvocationInterceptor {
    override fun interceptTestMethod(
        invocation: InvocationInterceptor.Invocation<Void>,
        invocationContext: ReflectiveInvocationContext<Method>,
        extensionContext: ExtensionContext
    ) {
        runInEdtAndWait {
            invocation.proceed()
        }
    }
}
