#include "SourceFileChainedCallbacks.h"

SourceFileChainedCallbacks::SourceFileChainedCallbacks(
    std::vector<std::unique_ptr<SourceFileCallbacks>> callbacks)
    : callbacks(std::move(callbacks)) {
}

bool SourceFileChainedCallbacks::handleBeginSource(clang::CompilerInstance &CI) {
    for (auto const &callback : callbacks) {
        if (!callback->handleBeginSource(CI)) {
            return false;
        }
    }
    return true;
}
void SourceFileChainedCallbacks::add(
    std::unique_ptr<clang::tooling::SourceFileCallbacks> callback) {
    callbacks.push_back(std::move(callback));
}
