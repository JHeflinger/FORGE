#include "Network.h"

grpc::Status Network::Connect(grpc::ServerContext* context, const ConnectionRequest* request, ConnectionResponse* response) {
    std::string client_ip = request->ip();
    std::string server_ip = context->peer();
    if (m_SimulationRef->RegisterClient(client_ip)) {
        response->set_ip(server_ip);
        return grpc::Status::OK;
    }
    return grpc::Status::CANCELLED;
}
