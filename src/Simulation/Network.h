#pragma once
#include "Simulation/Simulation.h"
#include "forge.grpc.pb.h"
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>

enum class PacketHeader {
	QUIT = 0,
	LOG,
	END,
	TOPOLOGY_RESPONSE,
	DISTRIBUTE_TOPOLOGY,
};

class Network final : public ForgeNet::Service {
public:
	Network(Simulation* context) : m_SimulationRef(context) {}
	~Network() {}
public:
	grpc::Status Connect(grpc::ServerContext* context, const ConnectionRequest* request, ConnectionResponse* response) override;
	grpc::Status Verify(grpc::ServerContext* context, const VerifyRequest* request, Empty* response) override;
	grpc::Status Communicate(grpc::ServerContext* context, grpc::ServerReaderWriter<GenericPacket, GenericPacket>* stream) override;
private:
	Simulation* m_SimulationRef = nullptr;
};
