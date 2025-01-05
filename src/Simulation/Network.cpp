#include "Network.h"

grpc::Status Network::Connect(grpc::ServerContext* context, const ConnectionRequest* request, ConnectionResponse* response) {
    std::string client_address = context->peer(); // Format: ipv4:127.0.0.1:port
	response->set_reply("Welcome to the network!");
    return grpc::Status::OK;
}
