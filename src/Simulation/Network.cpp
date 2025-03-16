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
    memset(&m_MainConnection.server_addr, 0, sizeof(struct sockaddr_in));
    m_MainConnection.server_addr.sin_family = AF_INET;
    m_MainConnection.server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &m_MainConnection.server_addr.sin_addr);
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

    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t client_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(50051);

    if (bind(m_MainConnection.sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        FATAL("Bind failed\n");
    }

    while (true) {
        char buffer[MAXIMUM_POSSIBLE_PACKET_LENGTH] = {0}; // TODO: find a better solution to doing this dynamically
        size_t received_bytes = recvfrom(m_MainConnection.sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);
        if (received_bytes < 0) {
            FATAL("Receive failed\n");
        }

        ClientInfo info;
        if (!info.ParseFromArray(buffer, received_bytes)) {
            WARN("Detected a corrupt packet");
        } else {
            std::string ipstr = info.ip();
            m_SimulationRef->RegisterClient(ipstr, 1);
        }
    }
}

void Network::ClientProcess() {

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
    std::string serialized_data;
    info.SerializeToString(&serialized_data);
    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.server_addr), sizeof(m_MainConnection.server_addr));
}

void Network::VerifyConnection() {

}

void Network::Close() {
    CLOSESOCK(m_MainConnection.sockfd);
}