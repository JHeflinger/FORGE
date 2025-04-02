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

#define SAFELY_GET_PACKET() { \
    result = select(m_MainConnection.sockfd + 1, &readfds, nullptr, nullptr, &timeout); \
    if (result < 0 || !(FD_ISSET(m_MainConnection.sockfd, &readfds))) continue; \
    received_bytes = recvfrom(m_MainConnection.sockfd, buffer, sizeof(buffer), 0, &other_addr, &other_len); \
    if (received_bytes < 0) { FATAL("Receive failed"); } }

#define WAIT_AND_GET_PACKET() { \
    received_bytes = recvfrom(m_MainConnection.sockfd, buffer, sizeof(buffer), 0, &other_addr, &other_len); \
    if (received_bytes < 0) { FATAL("Receive failed"); } }

#define SEND_ACK() { \
    AckPack ack; \
    ack.set_good(true); \
    std::string serialized_data; \
    ack.SerializeToString(&serialized_data); \
    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address)); }

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
    struct sockaddr other_addr{};
    socklen_t other_len = sizeof(other_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(50051);

    size_t pack_count = 0;

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
        char buffer[MAXIMUM_POSSIBLE_PACKET_LENGTH] = {0}; // TODO: find a better solution to doing this dynamically
        int result = 0;
        size_t received_bytes = 0;

        if (currstate == NetworkHostState::PREPARE) {
            SAFELY_GET_PACKET();
            ClientInfo info;
            if (!info.ParseFromArray(buffer, received_bytes)) {
                WARN("Detected a corrupt packet");
            } else {
                if (info.verify()) {
                    m_SimulationRef->VerifyClient(info.id());
                } else {
                    std::string ipstr = info.ip();
                    size_t cid = m_SimulationRef->RegisterClient(ipstr, 1);
                    m_ClientAddressPoints.push_back(other_addr);
                    ClientData data;
                    data.set_id(cid);
                    std::string serialized_data;
                    data.SerializeToString(&serialized_data);
                    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &other_addr, sizeof(other_addr));
                }
            }
        } else if (currstate == NetworkHostState::TOPOLOGIZE) {
            std::vector<ClientMetadata> clients = m_SimulationRef->Clients();
            for (int i = 0; i < clients.size(); i++) {
                TopologyInfo topi;
                topi.set_neighbor_id(i + 1);
                topi.set_neighbor_port(50051);
                topi.set_neighbor_ip(i == clients.size() - 1 ? "MAIN" : clients[i + 1].ip);
                std::string serialized_data;
                topi.SerializeToString(&serialized_data);
                sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
            }
            SetHostState(NetworkHostState::TOPOLOGIZE_RETURN);
            pack_count = 0;
        } else if (currstate == NetworkHostState::TOPOLOGIZE_RETURN) {
            SAFELY_GET_PACKET();
            pack_count++;
            if (pack_count >= m_SimulationRef->Clients().size()) {
                pack_count = 0;
                SetHostState(NetworkHostState::DISTRIBUTE);
            }
        } else if (currstate == NetworkHostState::DISTRIBUTE) {
            // TODO: ensure there are more workers than particles
            size_t num_workers = m_SimulationRef->Clients().size() + 1;
            std::vector<Ref<Particle>> particles = m_SimulationRef->Particles();
            bool uneven = particles.size() % num_workers != 0;
            size_t jobsize = uneven ? (particles.size() / num_workers) + 1 : particles.size() / num_workers;
            size_t optimal_workers = particles.size() % jobsize != 0 ? (particles.size() / jobsize) + 1 : particles.size() / jobsize;
            if (optimal_workers != num_workers) {
                WARN("more workers than possible jobs needed...");
            }
            for (uint32_t i = 0; i < m_SimulationRef->Clients().size(); i++) {
                size_t dataind = i * jobsize;
                size_t datasize = jobsize;
                if (i == num_workers - 1 && uneven) datasize = particles.size() % jobsize;
                for (uint32_t j = dataind; j < dataind + datasize; j++) {
                    ParticleTransfer p;
                    p.set_ax(particles[j]->Acceleration().x);
                    p.set_ay(particles[j]->Acceleration().y);
                    p.set_az(particles[j]->Acceleration().z);
                    p.set_px(particles[j]->Position().x);
                    p.set_py(particles[j]->Position().y);
                    p.set_pz(particles[j]->Position().z);
                    p.set_vx(particles[j]->Velocity().x);
                    p.set_vy(particles[j]->Velocity().y);
                    p.set_vz(particles[j]->Velocity().z);
                    p.set_mass(particles[j]->Mass());
                    p.set_package_size(datasize);
                    std::string serialized_data;
                    p.SerializeToString(&serialized_data);
                    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
                }
            }
            m_HostSliceInd = m_SimulationRef->Clients().size() * jobsize;
            m_HostSliceSize = jobsize;
            if (m_SimulationRef->Clients().size() == num_workers - 1 && uneven) m_HostSliceSize = particles.size() % jobsize;
            m_SimulationRef->SimulationRecord().clear();
            std::vector<Particle> slice;
            for (uint32_t i = m_HostSliceInd; i < m_HostSliceInd + m_HostSliceSize; i++) {
                slice.push_back(*particles[i]);
            }
            m_SimulationRef->SimulationRecord().push_back(slice);
            SetHostState(NetworkHostState::DISTRIBUTE_RETURN);
            pack_count = 0;
        } else if (currstate == NetworkHostState::DISTRIBUTE_RETURN) {
            SAFELY_GET_PACKET();
            pack_count++;
            if (pack_count >= m_SimulationRef->Clients().size()) {
                pack_count = 0;
                SetHostState(NetworkHostState::TREEPREP);
            }
        } else if (currstate == NetworkHostState::TREEPREP) {
            // TODO: recalculate bounds (perhaps get bounds from workers from prev awk step?)
			double xdif = ((m_SimulationRef->SchedulerReference()->bounds.xmax - m_SimulationRef->SchedulerReference()->bounds.xmin) / 2.0);
			double ydif	= ((m_SimulationRef->SchedulerReference()->bounds.ymax - m_SimulationRef->SchedulerReference()->bounds.ymin) / 2.0);
			double zdif	= ((m_SimulationRef->SchedulerReference()->bounds.zmax - m_SimulationRef->SchedulerReference()->bounds.zmin) / 2.0);
			Oct space = {
				xdif + m_SimulationRef->SchedulerReference()->bounds.xmin,
				ydif + m_SimulationRef->SchedulerReference()->bounds.ymin,
				zdif + m_SimulationRef->SchedulerReference()->bounds.zmin,
				(xdif > ydif ? (xdif > zdif ? xdif : zdif) : (ydif > zdif ? ydif : zdif))
			};
			size_t treesize = 0;
			size_t ignoreind = 0;
			Octtree tree(space, &treesize);
			while (treesize < m_SimulationRef->Clients().size() + 1) {
				tree.Insert(&(m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1][ignoreind]));
				ignoreind++;
			}
            std::vector<std::pair<Oct, Particle*>> plist;
            tree.AsList(&plist);
            for (size_t i = 0; i < m_SimulationRef->Clients().size(); i++) {
                TreeSeed ts;
                if (plist[i].second != nullptr) {
                    ts.set_px(plist[i].second->Position().x);
                    ts.set_py(plist[i].second->Position().y);
                    ts.set_pz(plist[i].second->Position().z);
                    ts.set_mass(plist[i].second->Mass());
                    ts.set_tx(plist[i].first.x);
                    ts.set_ty(plist[i].first.y);
                    ts.set_tz(plist[i].first.z);
                    ts.set_radius(plist[i].first.radius);
                    ts.set_empty(false);
                } else {
                    ts.set_empty(true);
                }
                std::string serialized_data;
                ts.SerializeToString(&serialized_data);
                sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
            }
            // for (size_t i = m_SimulationRef->Clients().size(); i < m_SimulationRef->Clients().size(); i++) {
            //     Scope<Octtree> tree = CreateScope<Octtree>(plist[i].first, nullptr);
            //     if (plist[i].second != nullptr) {
            //         m_Seed.SetPosition(plist[i].second->Position());
            //         m_Seed.SetMass(plist[i].second->Mass());
            //         tree->Insert(m_Seed);
            //     }
            //     m_Trees.push_back(tree);
            // }
            SetHostState(NetworkHostState::TREEDIST);
            pack_count = 0;
        } else if (currstate == NetworkHostState::TREEDIST) {
            INFO("made it 2");
        } else {
            WARN("No known network state being handled");
        }
    }
}

void Network::ClientProcess() {
    struct sockaddr other_addr{};
    socklen_t other_len = sizeof(other_addr);

    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_MainConnection.sockfd, &readfds);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000; // 1ms timeout
        m_SimulationRef->SchedulerReference()->lock.lock();
        NetworkClientState currstate = m_ClientState;
        m_SimulationRef->SchedulerReference()->lock.unlock();
        char buffer[MAXIMUM_POSSIBLE_PACKET_LENGTH] = {0}; // TODO: find a better solution to doing this dynamically
        int result = 0;
        size_t received_bytes = 0;
        if (currstate == NetworkClientState::EMPTY) {
            SAFELY_GET_PACKET();
            TopologyInfo info;
            if (!info.ParseFromArray(buffer, received_bytes)) {
                WARN("Detected a corrupt packet");
            } else {
                if (info.neighbor_ip() != "MAIN") {
                    m_NeighborID = info.neighbor_id();
                    m_Neighbor.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                    if (m_Neighbor.sockfd < 0) {
                        FATAL("Socket creation failed\n");
                    }
                    memset(&m_Neighbor.address, 0, sizeof(struct sockaddr_in));
                    m_Neighbor.address.sin_family = AF_INET;
                    m_Neighbor.address.sin_port = htons(info.neighbor_port());
                    inet_pton(AF_INET, info.neighbor_ip().c_str(), &m_Neighbor.address.sin_addr);
                } else {
                    m_NeighborID = (size_t)-1;
                }
                SEND_ACK();
            }
            SetClientState(NetworkClientState::RECEIVE_PARTICLES);
        } else if (currstate == NetworkClientState::RECEIVE_PARTICLES) {
            size_t particles_to_get = 1;
            m_SimulationRef->SchedulerReference()->lock.lock();
            m_SimulationRef->SimulationRecord().clear();
            m_SimulationRef->SchedulerReference()->lock.unlock();
            std::vector<Particle> slice;
            for (size_t i = 0; i < particles_to_get; i++) {
                WAIT_AND_GET_PACKET();
                ParticleTransfer p;
                if (!p.ParseFromArray(buffer, received_bytes)) {
                    WARN("Detected a corrupt packet");
                } else {
                    if (particles_to_get == 1) particles_to_get = p.package_size();
                    Particle rp;
                    rp.SetAcceleration({ p.ax(), p.ay(), p.az() });
                    rp.SetPosition({ p.px(), p.py(), p.pz() });
                    rp.SetVelocity({ p.vx(), p.vy(), p.vz() });
                    rp.SetMass(p.mass());
                    slice.push_back(rp);
                }
            }
            m_SimulationRef->SchedulerReference()->lock.lock();
            m_SimulationRef->SimulationRecord().push_back(slice);
            m_SimulationRef->SchedulerReference()->lock.unlock();
            SetClientState(NetworkClientState::RECEIVE_TREE);
            SEND_ACK();
        } else if (currstate == NetworkClientState::RECEIVE_TREE) {
            SAFELY_GET_PACKET();
            TreeSeed seed;
            if (!seed.ParseFromArray(buffer, received_bytes)) {
                WARN("Detected a corrupt packet");
            } else {
                // m_TreeSeeds.clear();
                // m_Trees.clear();
                // Particle p;
                // Oct o = {
                //     seed.tx(),
                //     seed.ty(),
                //     seed.tz(),
                //     seed.radius()
                // };
                // m_TreeSeeds.push_back(p);
                // Scope<Octtree> tree = CreateScope<Octtree>(o, nullptr);
                // if (!seed.empty()) {
                //     m_TreeSeeds[m_TreeSeeds.size() - 1].SetPosition({
                //         seed.px(),
                //         seed.py(),
                //         seed.pz(),
                //     });
                //     m_TreeSeeds[m_TreeSeeds.size() - 1].SetMass(seed.mass());
                //     tree->Insert(&m_TreeSeeds[m_TreeSeeds.size() - 1]);
                // }
                // m_Trees.push_back(tree);
            }
            SetClientState(NetworkClientState::CONSTRUCT_TREE);
        } else if (currstate == NetworkClientState::CONSTRUCT_TREE) {
            INFO("made it");
        } else {
            WARN("No known network state being handled");
        }
    }
}

void Network::SetHostState(NetworkHostState state) {
    std::unique_lock<std::mutex> lock(m_SimulationRef->SchedulerReference()->lock);
    m_HostState = state;
}

void Network::SetClientState(NetworkClientState state) {
    std::unique_lock<std::mutex> lock(m_SimulationRef->SchedulerReference()->lock);
    m_ClientState = state;
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