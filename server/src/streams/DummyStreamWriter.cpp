#include "DummyStreamWriter.h"

#include <optional>

bool DummyStreamWriter::hasStream() const {
    return false;
}

void DummyStreamWriter::writeProgress(const std::optional<std::string> &message,
                                      double percent,
                                      bool completed) const {
}

DummyStreamWriter instance;

ProgressWriter const *DummyStreamWriter::getInstance() {
    return &instance;
}
