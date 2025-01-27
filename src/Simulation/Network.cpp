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

grpc::Status Network::Communicate(grpc::ServerContext* context, grpc::ServerReaderWriter<GenericPacket, GenericPacket>* stream) {
    GenericPacket in_packet;
    std::vector<ClientMetadata> clients;
    while (true) {
        INFO("hey");
        uint32_t request = 0;
        if (m_SimulationRef->GetQueuedRequest(&request)) {
            PacketHeader purpose = (PacketHeader)request;
            switch (purpose) {
                case PacketHeader::DISTRIBUTE_TOPOLOGY:
                    clients = m_SimulationRef->Clients();
                    for (size_t i = 0; i < clients.size(); i++) {
                        GenericPacket out_packet;
                        out_packet.set_purpose((uint32_t)PacketHeader::TOPOLOGY_RESPONSE);
                        out_packet.set_strdata(clients[i].ip);
                        out_packet.set_idxdata(i);
                        stream->Write(out_packet);
                    }
                    break;
                default:
                    FATAL("Unknown request detected in server queue");
                    return grpc::Status::CANCELLED;
            }
        }
        if (stream->Read(&in_packet)) {
            PacketHeader purpose = (PacketHeader)in_packet.purpose();
            switch (purpose) {
                case PacketHeader::QUIT:
                    return grpc::Status::OK;
                    break;
                case PacketHeader::LOG:
                    m_SimulationRef->Log(in_packet.strdata());
                    break;
                // case PacketHeader::TOPOLOGY_REQUEST:
                //     clients = m_SimulationRef->Clients();
                //     for (size_t i = 0; i < clients.size(); i++) {
                //         GenericPacket out_packet;
                //         out_packet.set_purpose((uint32_t)PacketHeader::TOPOLOGY_RESPONSE);
                //         out_packet.set_strdata(clients[i].ip);
                //         out_packet.set_idxdata(i);
                //         stream->Write(out_packet);
                //     }
                //     break;
                default:
                    return grpc::Status::CANCELLED;
            }
            GenericPacket out_packet;
            out_packet.set_purpose((uint32_t)PacketHeader::END);
            stream->Write(out_packet);
        }
    }
    return grpc::Status::OK;
}