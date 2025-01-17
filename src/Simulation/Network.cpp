#include "Network.h"

grpc::Status Network::Connect(grpc::ServerContext* context, const ConnectionRequest* request, ConnectionResponse* response) {
    //std::string client_address = context->peer(); // Format: ipv4:127.0.0.1:port
    bool what = context->IsCancelled();
	response->set_reply("Hello World");
    return grpc::Status::OK;
}
