#pragma once
#include "Simulation/Simulation.h"
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server_context.h>
#include "forge.grpc.pb.h"

class Network final : public ForgeNet::Service {
public:
	Network(Simulation* context) : m_SimulationRef(context) {}
	~Network() {}
public:
	grpc::Status Connect(grpc::ServerContext* context, const ConnectionRequest* request, ConnectionResponse* response) override;
private:
	Simulation* m_SimulationRef = nullptr;
};
