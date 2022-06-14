#include "RequestEnvironment.h"

namespace RequestEnvironment {
    thread_local std::optional<std::string> clientId;
    thread_local grpc::ServerContext *serverContext;

    const std::string &getClientId() {
        if (!clientId.has_value()) {
            throw std::runtime_error(
                "Client id was not initialized. Did you forget to call \"setThreadOptions\"?");
        }
        return clientId.value();
    }

    const grpc::ServerContext *getServerContext() {
        return serverContext;
    }

    void setClientId(std::string requestClientId) {
        clientId = std::move(requestClientId);
    }

    void setServerContext(grpc::ServerContext *requestServerContext) {
        serverContext = requestServerContext;
    }

    bool isCancelled() {
        return serverContext && serverContext->IsCancelled();
    }
}
