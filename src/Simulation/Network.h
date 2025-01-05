#pragma once
#include <grpcpp/grpcpp.h>
#include "forge.grpc.pb.h"

class Network final : public ForgeNet::Service {
public:
	grpc::Status Connect(grpc::ServerContext* context, const ConnectionRequest* request, ConnectionResponse* response) override;
};
