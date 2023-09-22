#include "RequestEnvironment.h"

namespace RequestEnvironment {
    thread_local std::optional<std::string> clientId;
    thread_local grpc::ServerContext *serverContext;

    const std::string &getClientId() {
        if (!clientId.has_value()) {
            //TODO: Add more logs
            std::string message = "Client id was not initialized. Did you forget to call \"setThreadOptions\"?";
            throw std::runtime_error(message);
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
