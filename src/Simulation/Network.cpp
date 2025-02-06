#include "Network.h"
#include "Core/Log.h"

#ifdef _WIN32
static bool g_win_started = false;
#define CLOSESOCK(...) closesocket(__VA_ARGS__);
#else
#define CLOSESOCK(...) close(__VA_ARGS__);
#endif

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

void Network::Close() {
    CLOSESOCK(m_MainConnection.sockfd);
}