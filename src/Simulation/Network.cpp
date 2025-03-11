#include "Network.h"
#include "Core/Log.h"
#include "network.pb.h"

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

void Network::TestRecieve() {
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

    size_t count = 0;
    size_t evilcount = 0;
    size_t fatalcount = 0;
    while (true) {
        char buffer[1024] = {0};
        size_t received_bytes = recvfrom(m_MainConnection.sockfd, buffer, sizeof(buffer), 0,
                                        (struct sockaddr*)&client_addr, &client_len);
        count++;
        if (received_bytes < 0) {
            FATAL("Receive failed\n");
        }

        MyMessage msg;
        if (!msg.ParseFromArray(buffer, received_bytes)) {
            fatalcount++;
        } else {
            if (msg.text() != "This is my complex and cool string!" || count == 100000) {
                evilcount++;
                WARN("Detected packet loss - recieved \"{}\" instead of \"This is my complex and cool string!\". Overall detected {}/{} corrupt packets and {} fatal errors.", msg.text(), evilcount, count, fatalcount);
            }
        }
    }
}

void Network::TestSend() {
    MyMessage msg;
    msg.set_text("This is my complex and cool string!");
    std::string serialized_data;
    msg.SerializeToString(&serialized_data);
    while (true) {
        sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0,
            (struct sockaddr*)&(m_MainConnection.server_addr), sizeof(m_MainConnection.server_addr));
    }
}

void Network::Close() {
    CLOSESOCK(m_MainConnection.sockfd);
}