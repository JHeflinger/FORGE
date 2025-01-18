#include "Network.h"

grpc::Status Network::Connect(grpc::ServerContext* context, const ConnectionRequest* request, ConnectionResponse* response) {
    std::string client_ip = request->ip();
    std::string server_ip = context->peer();
    if (m_SimulationRef->RegisterClient(client_ip, request->local_workers())) {
        response->set_ip(server_ip);
        response->set_simulation_length(m_SimulationRef->Length());
        response->set_length_unit((uint32_t)(m_SimulationRef->LengthUnit()));
        response->set_enable_safeguard_cache(m_SimulationRef->SafeguardCacheEnabled());
        response->set_enable_simulation_record(m_SimulationRef->SimulationRecordEnabled());
        response->set_simulation_solver((uint32_t)(m_SimulationRef->Solver()));
        response->set_bounds_x(m_SimulationRef->Bounds().x);
        response->set_bounds_y(m_SimulationRef->Bounds().y);
        response->set_timestep(m_SimulationRef->Timestep());
        response->set_id(m_SimulationRef->ServerData().num_clients - 1);
        return grpc::Status::OK;
    }
    return grpc::Status::CANCELLED;
}

grpc::Status Network::Verify(grpc::ServerContext* context, const VerifyRequest* request, Empty* response) {
    m_SimulationRef->VerifyClient(request->id());
    return grpc::Status::OK;
}