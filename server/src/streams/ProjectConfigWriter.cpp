#include "ProjectConfigWriter.h"

void ProjectConfigWriter::writeResponse(testsgen::ProjectConfigStatus status,
                                        const std::optional<std::string> &message) const {
    testsgen::ProjectConfigResponse response;
    response.set_type(status);
    if (message.has_value()) {
        response.set_message(message.value());
    }
    writeMessage(response);
}
