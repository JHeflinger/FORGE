#include "Network.h"
#include "Core/Log.h"
#include "network.pb.h"

#ifdef _WIN32
static bool g_win_started = false;
#define CLOSESOCK(...) closesocket(__VA_ARGS__);
#else
#define CLOSESOCK(...) close(__VA_ARGS__);
#endif

#define MAXIMUM_POSSIBLE_PACKET_LENGTH 4096

Network::~Network() {
    #ifdef _WIN32
    WSACleanup();
    #endif
}

void Network::Open(std::string ip, uint16_t port) {
    #ifdef _WIN32
    if (!g_win_started) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            FATAL("WSAStartup failed\n");
        }
    }
    #endif
    m_MainConnection.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_MainConnection.sockfd < 0) {
        FATAL("Socket creation failed\n");
    }
    memset(&m_MainConnection.address, 0, sizeof(struct sockaddr_in));
    m_MainConnection.address.sin_family = AF_INET;
    m_MainConnection.address.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &m_MainConnection.address.sin_addr);
}

void Network::HostProcess() {
    #ifdef _WIN32
    if (!g_win_started) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            FATAL("WSAStartup failed\n");
        }
    }
    #endif
    m_MainConnection.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_MainConnection.sockfd < 0) {
        FATAL("Socket creation failed\n");
    }

    struct sockaddr_in server_addr{};
    struct sockaddr client_addr{};
    socklen_t client_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(50051);

    if (bind(m_MainConnection.sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        FATAL("Bind failed\n");
    }

    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_MainConnection.sockfd, &readfds);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000; // 1ms timeout
        m_SimulationRef->SchedulerReference()->lock.lock();
        NetworkHostState currstate = m_HostState;
        m_SimulationRef->SchedulerReference()->lock.unlock();
        if (currstate == NetworkHostState::PREPARE) {
            int result = select(m_MainConnection.sockfd + 1, &readfds, nullptr, nullptr, &timeout);
            if (result > 0 && FD_ISSET(m_MainConnection.sockfd, &readfds)) {
                char buffer[MAXIMUM_POSSIBLE_PACKET_LENGTH] = {0}; // TODO: find a better solution to doing this dynamically
                size_t received_bytes = recvfrom(m_MainConnection.sockfd, buffer, sizeof(buffer), 0, &client_addr, &client_len);
                if (received_bytes < 0) {
                    FATAL("Receive failed\n");
                }

                ClientInfo info;
                if (!info.ParseFromArray(buffer, received_bytes)) {
                    WARN("Detected a corrupt packet");
                } else {
                    if (info.verify()) {
                        m_SimulationRef->VerifyClient(info.id());
                    } else {
                        std::string ipstr = info.ip();
                        size_t cid = m_SimulationRef->RegisterClient(ipstr, 1);
                        m_ClientAddressPoints.push_back(client_addr);
                        ClientData data;
                        data.set_id(cid);
                        std::string serialized_data;
                        data.SerializeToString(&serialized_data);
                        sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &client_addr, sizeof(client_addr));
                    }
                }
            }
        } else if (currstate == NetworkHostState::TOPOLOGIZE) {
            std::vector<ClientMetadata> clients = m_SimulationRef->Clients();
            for (int i = 0; i < clients.size(); i++) {
                size_t neighbor_id = i == clients.size() - 1 ? 0 : i + 1;
                TopologyInfo topi;
                topi.set_neighbor_id((uint64_t)neighbor_id);
                topi.set_neighbor_port((uint32_t)m_MainConnection.port);
                topi.set_neighbor_ip(clients[neighbor_id].ip);
                std::string serialized_data;
                topi.SerializeToString(&serialized_data);
                sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[neighbor_id]), sizeof(m_ClientAddressPoints[neighbor_id]));
            }
        } else {
            WARN("No known network state being handled");
        }
    }
}

void Network::ClientProcess() {
    while (true) {
        
    }
}

void Network::SetState(NetworkHostState state) {
    std::unique_lock<std::mutex> lock(m_SimulationRef->SchedulerReference()->lock);
    m_HostState = state;
}

void Network::SendNetworkInfo() {
    char hostname[256];
    char hostip[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent *host = gethostbyname(hostname);
    sprintf(hostip, "%d.%d.%d.%d",
        (int)((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b1,
        (int)((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b2,
        (int)((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b3,
        (int)((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b4);
    ClientInfo info;
    info.set_ip(std::string(hostip));
    info.set_id(0);
    info.set_verify(false);
    std::string serialized_data;
    info.SerializeToString(&serialized_data);
    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
}

void Network::VerifyConnection() {
    ClientInfo info;
    info.set_ip("");
    info.set_id(m_ClientID);
    info.set_verify(true);
    std::string serialized_data;
    info.SerializeToString(&serialized_data);
    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
}

void Network::ReceiveID() {
    char buffer[256] = {0};
    struct sockaddr_in recv_addr{};
    socklen_t recv_len = sizeof(recv_addr);

    int received_bytes = recvfrom(m_MainConnection.sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&recv_addr, &recv_len);
    if (received_bytes >= 0) {
        ClientData response;
        response.ParseFromArray(buffer, received_bytes);
        m_ClientID = (size_t)response.id();
    } else {
        WARN("Detected a corrupt packet");
    }
}

void Network::Close() {
    CLOSESOCK(m_MainConnection.sockfd);
}