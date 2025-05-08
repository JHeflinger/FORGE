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
    int socksize = 1 << 25; // 1 MB
    setsockopt(m_MainConnection.sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&socksize, sizeof(socksize));
    setsockopt(m_MainConnection.sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&socksize, sizeof(socksize));
    memset(&m_MainConnection.address, 0, sizeof(struct sockaddr_in));
    m_MainConnection.address.sin_family = AF_INET;
    m_MainConnection.address.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &m_MainConnection.address.sin_addr);
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(50051);
    if (bind(m_MainConnection.sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        FATAL("Bind failed\n");
    }
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
    int socksize = 1 << 25; // 1 MB
    setsockopt(m_MainConnection.sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&socksize, sizeof(socksize));
    setsockopt(m_MainConnection.sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&socksize, sizeof(socksize));

    struct sockaddr_in server_addr{};
    struct sockaddr other_addr{};
    socklen_t other_len = sizeof(other_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(50051);

    size_t pack_count = 0;
    size_t returned_particles = 0;
    size_t particles_waiting_on = 0;

	uint64_t total_steps = m_SimulationRef->Length() / m_SimulationRef->Timestep();
    uint64_t current_steps = 0;

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

        if (current_steps >= total_steps) {
            m_SimulationRef->Finish();
            for (size_t i = 0; i < m_SimulationRef->Clients().size(); i++) {
                TreeSeed ts;
                ts.set_end_sim(true);
                std::string serialized_data;
                ts.SerializeToString(&serialized_data);
                sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
            }
            pack_count = 0;
            while (pack_count < m_SimulationRef->Clients().size()) {
                FD_ZERO(&readfds);
                FD_SET(m_MainConnection.sockfd, &readfds);
                timeout.tv_sec = 0;
                timeout.tv_usec = 1000; // 1ms timeout
                memset(buffer, 0, MAXIMUM_POSSIBLE_PACKET_LENGTH);
                SAFELY_GET_PACKET();
                CompileStage cs;
                if (!cs.ParseFromArray(buffer, received_bytes)) {
                    WARN("Detected a corrupt packet");
                } else {
                    if (cs.finished()) pack_count++;
                    Particle p;
                    p.SetPosition({cs.px(), cs.py(), cs.pz()});
                    m_SimulationRef->SimulationRecord()[cs.index()].push_back(p);
                }
            }
            return;
        }

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
			double xdif = ((m_SimulationRef->SchedulerReference()->bounds.xmax - m_SimulationRef->SchedulerReference()->bounds.xmin) / 2.0);
			double ydif	= ((m_SimulationRef->SchedulerReference()->bounds.ymax - m_SimulationRef->SchedulerReference()->bounds.ymin) / 2.0);
			double zdif	= ((m_SimulationRef->SchedulerReference()->bounds.zmax - m_SimulationRef->SchedulerReference()->bounds.zmin) / 2.0);
			Oct space = {
				xdif + m_SimulationRef->SchedulerReference()->bounds.xmin,
				ydif + m_SimulationRef->SchedulerReference()->bounds.ymin,
				zdif + m_SimulationRef->SchedulerReference()->bounds.zmin,
				(xdif > ydif ? (xdif > zdif ? xdif : zdif) : (ydif > zdif ? ydif : zdif))
			};
            m_SimulationRef->SchedulerReference()->bounds.Reset();
			size_t treesize = 0;
			size_t ignoreind = 0;
			Octtree tree(space, &treesize);
			while (treesize < m_SimulationRef->Clients().size() + 1) {
				tree.Insert(&(m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1][ignoreind]));
				ignoreind++;
			}
            m_IgnoreThreshold = ignoreind;
            std::vector<std::pair<Oct, Particle*>> plist;
            tree.AsList(&plist);
            for (size_t i = 0; i < m_SimulationRef->Clients().size(); i++) {
                TreeSeed ts;
                ts.set_tx(plist[i].first.x);
                ts.set_ty(plist[i].first.y);
                ts.set_tz(plist[i].first.z);
                ts.set_radius(plist[i].first.radius);
                if (plist[i].second != nullptr) {
                    ts.set_px(plist[i].second->Position().x);
                    ts.set_py(plist[i].second->Position().y);
                    ts.set_pz(plist[i].second->Position().z);
                    ts.set_mass(plist[i].second->Mass());
                    ts.set_empty(false);
                } else {
                    ts.set_empty(true);
                }
                std::string serialized_data;
                ts.SerializeToString(&serialized_data);
                sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
            }
            m_Trees.size = 0;
            m_Seeds.clear();
            for (size_t i = m_SimulationRef->Clients().size(); i < plist.size(); i++) {
                m_Trees.trees[m_Trees.size] = Octtree(plist[i].first, nullptr);
                if (plist[i].second != nullptr) {
                    m_Seeds.emplace_back();
                    m_Seeds[m_Seeds.size() - 1].SetPosition(plist[i].second->Position());
                    m_Seeds[m_Seeds.size() - 1].SetMass(plist[i].second->Mass());
                    m_Trees.trees[m_Trees.size].Insert(&m_Seeds[m_Seeds.size() - 1]);
                }
                m_Trees.size++;
            }
            SetHostState(NetworkHostState::TREEPREP_RETURN);
            pack_count = 0;
        } else if (currstate == NetworkHostState::TREEPREP_RETURN) {
            SAFELY_GET_PACKET();
            pack_count++;
            if (pack_count >= m_SimulationRef->Clients().size()) {
                pack_count = 0;
                SetHostState(NetworkHostState::TREEDIST);
                for (size_t i = 0; i < m_SimulationRef->Clients().size(); i++) {
                    TreeStage ts;
                    ts.set_approve(true);
                    std::string serialized_data;
                    ts.SerializeToString(&serialized_data);
                    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
                }
            }
        } else if (currstate == NetworkHostState::TREEDIST) {
            particles_waiting_on = 0;
            std::vector<Particle>* pslice = &(m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1]);
            for (size_t i = m_IgnoreThreshold; i < pslice->size(); i++) {
                bool contained = false;
                for (size_t j = 0; j < m_Trees.size; j++) {
                    if (m_Trees.trees[j].Contains(&(*pslice)[i])) {
                        m_Trees.trees[j].Insert(&(*pslice)[i]);
                        contained = true;
                        break;
                    }
                }
                if (!contained) {
                    particles_waiting_on++;
                    TreeStage ts;
                    ts.set_origin_id((uint64_t)-1);
                    ts.set_px((*pslice)[i].Position().x);
                    ts.set_py((*pslice)[i].Position().y);
                    ts.set_pz((*pslice)[i].Position().z);
                    ts.set_mass((*pslice)[i].Mass());
                    std::string serialized_data;
                    ts.SerializeToString(&serialized_data);
                    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[0]), sizeof(m_ClientAddressPoints[0]));
                }
            }
            SetHostState(NetworkHostState::TREEBUILD);
            pack_count = 0;
            returned_particles = 0;
        } else if (currstate == NetworkHostState::TREEBUILD) {
            SAFELY_GET_PACKET();
            TreeStage ts;
            if (!ts.ParseFromArray(buffer, received_bytes)) {
                WARN("Detected a corrupt packet");
            } else {
                if (ts.finished()) {
                    pack_count++;
                } else {
                    if (ts.origin_id() == ((uint64_t)-1)) {
                        returned_particles++;
                    } else {
                        if (!ts.added()) {
                            Particle p;
                            p.SetPosition({ts.px(), ts.py(), ts.pz()});
                            p.SetMass(ts.mass());
                            for (size_t j = 0; j < m_Trees.size; j++) {
                                if (m_Trees.trees[j].Contains(&p)) {
                                    m_Seeds.push_back(p);
                                    m_Trees.trees[j].Insert(&m_Seeds[m_Seeds.size() - 1]);
                                    ts.set_added(true);
                                    break;
                                }
                            }
                        }
                        std::string serialized_data;
                        ts.SerializeToString(&serialized_data);
                        sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[0]), sizeof(m_ClientAddressPoints[0]));
                    }
                }
            }
            if ((returned_particles >= particles_waiting_on) && pack_count >= m_SimulationRef->Clients().size()) {
                returned_particles = 0;
                pack_count = 0;
                SetHostState(NetworkHostState::TREEBUILD_RETURN);
                for (size_t j = 0; j < m_Trees.size; j++) {
			        m_Trees.trees[j].CalculateCenterOfMass();
                }
                for (size_t i = 0; i < m_SimulationRef->Clients().size(); i++) {
                    TreeStage stopcommand;
                    stopcommand.set_stop(true);
                    std::string serialized_data;
                    stopcommand.SerializeToString(&serialized_data);
                    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
                }
            }
        } else if (currstate == NetworkHostState::TREEBUILD_RETURN) {
            SAFELY_GET_PACKET();
            pack_count++;
            if (pack_count >= m_SimulationRef->Clients().size()) {
                pack_count = 0;
                SetHostState(NetworkHostState::EVALUATE);
                for (size_t i = 0; i < m_SimulationRef->Clients().size(); i++) {
                    EvaluateStage es;
                    es.set_approve(true);
                    std::string serialized_data;
                    es.SerializeToString(&serialized_data);
                    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
                }
                std::vector<Particle>* pslice = &(m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1]);
                for (size_t i = 0; i < pslice->size(); i++) {
                    Particle p = (*pslice)[i];
					p.SetVelocity(p.Velocity() + (0.5 * m_SimulationRef->Timestep() * p.Acceleration())/m_SimulationRef->UnitSize());
                    p.SetAcceleration({ 0.0, 0.0, 0.0 });
                    for (size_t j = 0; j < m_Trees.size; j++) {
                        m_Trees.trees[j].SerialCalculateForce(p, m_SimulationRef->UnitSize());
                    }
                    EvaluateStage es;
                    es.set_origin_id(((uint64_t)-1));
                    es.set_px(p.Position().x);
                    es.set_py(p.Position().y);
                    es.set_pz(p.Position().z);
                    es.set_vx(p.Velocity().x);
                    es.set_vy(p.Velocity().y);
                    es.set_vz(p.Velocity().z);
                    es.set_ax(p.Acceleration().x);
                    es.set_ay(p.Acceleration().y);
                    es.set_az(p.Acceleration().z);
                    es.set_mass(p.Mass());
                    std::string serialized_data;
                    es.SerializeToString(&serialized_data);
                    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[0]), sizeof(m_ClientAddressPoints[0]));
                }
                m_SimulationRef->SimulationRecord().emplace_back();
            }
        } else if (currstate == NetworkHostState::EVALUATE) {
            SAFELY_GET_PACKET();
            EvaluateStage es;
            if (!es.ParseFromArray(buffer, received_bytes)) {
                WARN("Detected a corrupt packet");
            } else {
                if (es.finished()) {
                    pack_count++;
                    if (es.px() < m_SimulationRef->SchedulerReference()->bounds.xmin)
                        m_SimulationRef->SchedulerReference()->bounds.xmin = es.px();
                    if (es.py() < m_SimulationRef->SchedulerReference()->bounds.ymin)
                        m_SimulationRef->SchedulerReference()->bounds.ymin = es.py();
                    if (es.pz() < m_SimulationRef->SchedulerReference()->bounds.zmin)
                        m_SimulationRef->SchedulerReference()->bounds.zmin = es.pz();
                    if (es.vx() > m_SimulationRef->SchedulerReference()->bounds.xmax)
                        m_SimulationRef->SchedulerReference()->bounds.xmax = es.vx();
                    if (es.vy() > m_SimulationRef->SchedulerReference()->bounds.ymax)
                        m_SimulationRef->SchedulerReference()->bounds.ymax = es.vy();
                    if (es.vz() > m_SimulationRef->SchedulerReference()->bounds.zmax)
                        m_SimulationRef->SchedulerReference()->bounds.zmax = es.vz();
                } else {
                    Particle p;
                    p.SetPosition({es.px(), es.py(), es.pz()});
                    p.SetVelocity({es.vx(), es.vy(), es.vz()});
                    p.SetAcceleration({es.ax(), es.ay(), es.az()});
                    p.SetMass(es.mass());
                    if (es.origin_id() == ((uint64_t)-1)) {
					    p.SetVelocity(p.Velocity() + (0.5 * m_SimulationRef->Timestep() * p.Acceleration())/m_SimulationRef->UnitSize());
					    p.SetPosition(p.Position() + ((double)m_SimulationRef->Timestep() * p.Velocity()));
                        if (p.Position().x < m_SimulationRef->SchedulerReference()->bounds.xmin) m_SimulationRef->SchedulerReference()->bounds.xmin = p.Position().x;
                        if (p.Position().y < m_SimulationRef->SchedulerReference()->bounds.ymin) m_SimulationRef->SchedulerReference()->bounds.ymin = p.Position().y;
                        if (p.Position().z < m_SimulationRef->SchedulerReference()->bounds.zmin) m_SimulationRef->SchedulerReference()->bounds.zmin = p.Position().z;
                        if (p.Position().x > m_SimulationRef->SchedulerReference()->bounds.xmax) m_SimulationRef->SchedulerReference()->bounds.xmax = p.Position().x;
                        if (p.Position().y > m_SimulationRef->SchedulerReference()->bounds.ymax) m_SimulationRef->SchedulerReference()->bounds.ymax = p.Position().y;
                        if (p.Position().z > m_SimulationRef->SchedulerReference()->bounds.zmax) m_SimulationRef->SchedulerReference()->bounds.zmax = p.Position().z;
                        m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1].push_back(p);
                    } else {
                        for (size_t j = 0; j < m_Trees.size; j++) {
                            m_Trees.trees[j].SerialCalculateForce(p, m_SimulationRef->UnitSize());
                        }
                        es.set_px(p.Position().x);
                        es.set_py(p.Position().y);
                        es.set_pz(p.Position().z);
                        es.set_vx(p.Velocity().x);
                        es.set_vy(p.Velocity().y);
                        es.set_vz(p.Velocity().z);
                        es.set_ax(p.Acceleration().x);
                        es.set_ay(p.Acceleration().y);
                        es.set_az(p.Acceleration().z);
                        es.set_mass(p.Mass());
                        std::string serialized_data;
                        es.SerializeToString(&serialized_data);
                        sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[0]), sizeof(m_ClientAddressPoints[0]));
                    }
                }
            }
            if ((m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1].size() ==
                m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 2].size()) &&
                pack_count >= m_SimulationRef->Clients().size()) {
                pack_count = 0;
                SetHostState(NetworkHostState::EVALUATE_RETURN);
                for (size_t i = 0; i < m_SimulationRef->Clients().size(); i++) {
                    EvaluateStage stopcommand;
                    stopcommand.set_stop(true);
                    std::string serialized_data;
                    stopcommand.SerializeToString(&serialized_data);
                    sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, &(m_ClientAddressPoints[i]), sizeof(m_ClientAddressPoints[i]));
                }
            }
        } else if (currstate == NetworkHostState::EVALUATE_RETURN) {
            SAFELY_GET_PACKET();
            pack_count++;
            if (pack_count >= m_SimulationRef->Clients().size()) {
                pack_count = 0;
                SetHostState(NetworkHostState::TREEPREP);
                current_steps++;
                m_SimulationRef->UpdateProgress((float)((float)(current_steps + 1) / (float)total_steps));
            }
        } else {
            WARN("No known network state being handled");
        }
    }
}

void Network::ClientProcess() {
    struct sockaddr other_addr{};
    socklen_t other_len = sizeof(other_addr);

    std::vector<Particle> particles_to_send;
    size_t returned_particles = 0;
    size_t particles_waiting_on = 0;

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
                    int socksize = 1 << 20; // 1 MB
                    setsockopt(m_Neighbor.sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&socksize, sizeof(socksize));
                    setsockopt(m_Neighbor.sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&socksize, sizeof(socksize));
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
                if (seed.end_sim()) {
                    for (size_t i = 0; i < m_SimulationRef->SimulationRecord().size(); i++) {
                        for (size_t j = 0; j < m_SimulationRef->SimulationRecord()[i].size(); j++) {
                            CompileStage cs;
                            if (i == m_SimulationRef->SimulationRecord().size() - 1 && j == m_SimulationRef->SimulationRecord()[i].size() - 1)
                                cs.set_finished(true);
                            cs.set_px(m_SimulationRef->SimulationRecord()[i][j].Position().x);
                            cs.set_py(m_SimulationRef->SimulationRecord()[i][j].Position().y);
                            cs.set_pz(m_SimulationRef->SimulationRecord()[i][j].Position().z);
                            cs.set_index((uint64_t)i);
                            std::string serialized_data;
                            cs.SerializeToString(&serialized_data);
                            sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
                        }
                    }
                    return;
                }
                m_Trees.size = 1;
                m_Seeds.clear();
                Oct o = {
                    seed.tx(),
                    seed.ty(),
                    seed.tz(),
                    seed.radius()
                };
                m_Trees.trees[0] = Octtree(o, nullptr);
                if (!seed.empty()) {
                    m_Seeds.emplace_back();
                    m_Seeds[0].SetPosition({seed.px(), seed.py(), seed.pz()});
                    m_Seeds[0].SetMass(seed.mass());
                    m_Trees.trees[0].Insert(&(m_Seeds[0]));
                }
            }
            m_SimulationRef->SchedulerReference()->bounds.Reset();
            SetClientState(NetworkClientState::CONSTRUCT_TREE);
            SEND_ACK();
            particles_to_send.clear();
            particles_waiting_on = 0;
            std::vector<Particle>* pslice = &(m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1]);
            for (size_t i = 0; i < pslice->size(); i++) {
                bool contained = false;
                for (size_t j = 0; j < m_Trees.size; j++) {
                    if (m_Trees.trees[j].Contains(&(*pslice)[i])) {
                        m_Trees.trees[j].Insert(&(*pslice)[i]);
                        contained = true;
                        break;
                    }
                }
                if (!contained) {
                    particles_waiting_on++;
                    particles_to_send.push_back((*pslice)[i]);
                }
            }
            returned_particles = 0;
        } else if (currstate == NetworkClientState::CONSTRUCT_TREE) {
            SAFELY_GET_PACKET();
            TreeStage ts;
            if (!ts.ParseFromArray(buffer, received_bytes)) {
                WARN("Detected a corrupt packet");
            } else {
                if (ts.approve()) {
                    for (size_t i = 0; i < particles_to_send.size(); i++) {
                        TreeStage tout;
                        tout.set_origin_id(m_ClientID);
                        tout.set_px((particles_to_send)[i].Position().x);
                        tout.set_py((particles_to_send)[i].Position().y);
                        tout.set_pz((particles_to_send)[i].Position().z);
                        tout.set_mass((particles_to_send)[i].Mass());
                        std::string serialized_data;
                        tout.SerializeToString(&serialized_data);
                        if (m_NeighborID != ((size_t)-1)) {
                            sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_Neighbor.address), sizeof(m_Neighbor.address));
                        } else {
                            sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
                        }
                    }
                } else if (ts.stop()) {
                    SetClientState(NetworkClientState::EVALUATE_PARTICLES);
                    SEND_ACK();
                    m_SimulationRef->SimulationRecord().emplace_back();
                    for (size_t j = 0; j < m_Trees.size; j++) {
                        m_Trees.trees[j].CalculateCenterOfMass();
                    }
                } else {
                    if (ts.origin_id() == m_ClientID) {
                        returned_particles++;
                        if (returned_particles >= particles_waiting_on) {
                            TreeStage finishcommand;
                            finishcommand.set_finished(true);
                            std::string serialized_data;
                            finishcommand.SerializeToString(&serialized_data);
                            sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
                        }
                    } else {
                        if (!ts.added()) {
                            Particle p;
                            p.SetPosition({ts.px(), ts.py(), ts.pz()});
                            p.SetMass(ts.mass());
                            for (size_t j = 0; j < m_Trees.size; j++) {
                                if (m_Trees.trees[j].Contains(&p)) {
                                    m_Seeds.push_back(p);
                                    m_Trees.trees[j].Insert(&m_Seeds[m_Seeds.size() - 1]);
                                    ts.set_added(true);
                                    break;
                                }
                            }
                        }
                        std::string serialized_data;
                        ts.SerializeToString(&serialized_data);
                        if (m_NeighborID != ((size_t)-1)) {
                            sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_Neighbor.address), sizeof(m_Neighbor.address));
                        } else {
                            sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
                        }
                    }
                }
            }
        } else if (currstate == NetworkClientState::EVALUATE_PARTICLES) {
            // TODO: communicate over simulation config
            SAFELY_GET_PACKET();
            EvaluateStage es;
            if (!es.ParseFromArray(buffer, received_bytes)) {
                WARN("Detected a corrupt packet");
            } else {
                if (es.approve()) {
                    std::vector<Particle>* pslice = &(m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 2]);
                    for (size_t i = 0; i < pslice->size(); i++) {
                        Particle p = (*pslice)[i];
					    p.SetVelocity(p.Velocity() + (0.5 * m_SimulationRef->Timestep() * p.Acceleration())/m_SimulationRef->UnitSize());
                        p.SetAcceleration({ 0.0, 0.0, 0.0 });
                        for (size_t j = 0; j < m_Trees.size; j++) {
                            m_Trees.trees[j].SerialCalculateForce(p, m_SimulationRef->UnitSize());
                        }
                        EvaluateStage es_out;
                        es_out.set_origin_id(m_ClientID);
                        es_out.set_px(p.Position().x);
                        es_out.set_py(p.Position().y);
                        es_out.set_pz(p.Position().z);
                        es_out.set_vx(p.Velocity().x);
                        es_out.set_vy(p.Velocity().y);
                        es_out.set_vz(p.Velocity().z);
                        es_out.set_ax(p.Acceleration().x);
                        es_out.set_ay(p.Acceleration().y);
                        es_out.set_az(p.Acceleration().z);
                        es_out.set_mass(p.Mass());
                        std::string serialized_data;
                        es_out.SerializeToString(&serialized_data);
                        if (m_NeighborID != ((size_t)-1)) {
                            sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_Neighbor.address), sizeof(m_Neighbor.address));
                        } else {
                            sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
                        }
                    }
                } else {
                    if (es.stop()) {
                        SetClientState(NetworkClientState::RECEIVE_TREE);
                        SEND_ACK();
                    } else {
                        Particle p;
                        p.SetPosition({es.px(), es.py(), es.pz()});
                        p.SetVelocity({es.vx(), es.vy(), es.vz()});
                        p.SetAcceleration({es.ax(), es.ay(), es.az()});
                        p.SetMass(es.mass());
                        if (es.origin_id() == m_ClientID) {
					        p.SetVelocity(p.Velocity() + (0.5 * m_SimulationRef->Timestep() * p.Acceleration())/m_SimulationRef->UnitSize());
					        p.SetPosition(p.Position() + ((double)m_SimulationRef->Timestep() * p.Velocity()));
                            if (p.Position().x < m_SimulationRef->SchedulerReference()->bounds.xmin) m_SimulationRef->SchedulerReference()->bounds.xmin = p.Position().x;
                            if (p.Position().y < m_SimulationRef->SchedulerReference()->bounds.ymin) m_SimulationRef->SchedulerReference()->bounds.ymin = p.Position().y;
                            if (p.Position().z < m_SimulationRef->SchedulerReference()->bounds.zmin) m_SimulationRef->SchedulerReference()->bounds.zmin = p.Position().z;
                            if (p.Position().x > m_SimulationRef->SchedulerReference()->bounds.xmax) m_SimulationRef->SchedulerReference()->bounds.xmax = p.Position().x;
                            if (p.Position().y > m_SimulationRef->SchedulerReference()->bounds.ymax) m_SimulationRef->SchedulerReference()->bounds.ymax = p.Position().y;
                            if (p.Position().z > m_SimulationRef->SchedulerReference()->bounds.zmax) m_SimulationRef->SchedulerReference()->bounds.zmax = p.Position().z;
                            m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1].push_back(p);
                            if (m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 1].size() ==
                                m_SimulationRef->SimulationRecord()[m_SimulationRef->SimulationRecord().size() - 2].size()) {
                                EvaluateStage es_out;
                                es_out.set_finished(true);
                                es_out.set_px(m_SimulationRef->SchedulerReference()->bounds.xmin); // min bound x
                                es_out.set_vx(m_SimulationRef->SchedulerReference()->bounds.xmax); // max bound x
                                es_out.set_py(m_SimulationRef->SchedulerReference()->bounds.ymin); // min bound y
                                es_out.set_vy(m_SimulationRef->SchedulerReference()->bounds.ymax); // max bound y
                                es_out.set_pz(m_SimulationRef->SchedulerReference()->bounds.zmin); // min bound z
                                es_out.set_vz(m_SimulationRef->SchedulerReference()->bounds.zmax); // max bound z
                                std::string serialized_data;
                                es_out.SerializeToString(&serialized_data);
                                sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
                            }
                        } else {
                            for (size_t j = 0; j < m_Trees.size; j++) {
                                m_Trees.trees[j].SerialCalculateForce(p, m_SimulationRef->UnitSize());
                            }
                            es.set_px(p.Position().x);
                            es.set_py(p.Position().y);
                            es.set_pz(p.Position().z);
                            es.set_vx(p.Velocity().x);
                            es.set_vy(p.Velocity().y);
                            es.set_vz(p.Velocity().z);
                            es.set_ax(p.Acceleration().x);
                            es.set_ay(p.Acceleration().y);
                            es.set_az(p.Acceleration().z);
                            es.set_mass(p.Mass());
                            std::string serialized_data;
                            es.SerializeToString(&serialized_data);
                            if (m_NeighborID != ((size_t)-1)) {
                                sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_Neighbor.address), sizeof(m_Neighbor.address));
                            } else {
                                sendto(m_MainConnection.sockfd, serialized_data.data(), serialized_data.size(), 0, (struct sockaddr*)&(m_MainConnection.address), sizeof(m_MainConnection.address));
                            }
                        }
                    }
                }
            }            
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
    struct sockaddr_in dest;
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    inet_pton(AF_INET, "8.8.8.8", &dest.sin_addr);
    connect(sock, (struct sockaddr*)&dest, sizeof(dest));
    getsockname(sock, (struct sockaddr*)&name, &namelen);
    uint8_t ipv4[4];
	memcpy(ipv4, &name.sin_addr.s_addr, 4);
    CLOSESOCK(sock);
    char hostip[256];
    sprintf(hostip, "%d.%d.%d.%d", (int)ipv4[0], (int)ipv4[1], (int)ipv4[2], (int)ipv4[3]);
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