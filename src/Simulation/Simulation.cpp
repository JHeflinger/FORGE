#include "Simulation.h"
#include "Simulation/Network.h"
#include "Core/Log.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <cmath>

#define EPS 0.0000000000001 // epsilon for numerical stability
#define TIMENOW() (uint64_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count()

std::string GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&currentTime);
    std::stringstream timeStream;
    timeStream << '[' 
               << std::setw(2) << std::setfill('0') << localTime.tm_hour << ':'
               << std::setw(2) << std::setfill('0') << localTime.tm_min << ':'
               << std::setw(2) << std::setfill('0') << localTime.tm_sec << ':'
			   << std::setw(3) << std::setfill('0') << millis.count() << ']';
    return timeStream.str();
}

void Simulation::Log(std::string log) {
    m_Logs.push_back(GetCurrentTimeString() + " " + log); 
    if (m_Logs.size() > 10000) 
        m_Logs.erase(m_Logs.begin());
}

bool Simulation::Started() {
    return m_Started; 
}

bool Simulation::Paused() {
    return m_Paused; 
}

void Simulation::Simulate() {
	#define WAIT_ON_WORKERS() for (size_t j = 0; j < m_Scheduler.metadata.size(); j++) { \
		std::unique_lock<std::mutex> lock(m_Scheduler.lock); \
		m_Scheduler.controller_alert.wait(lock, [this, &j] { return m_Scheduler.metadata[j].finished; }); \
	}
	#define LAUNCH_NAIVE_STEP(step)	m_Scheduler.lock.lock(); for (size_t j = 0; j < m_Scheduler.metadata.size(); j++) { \
		m_Scheduler.metadata[j].finished = false; \
		m_Scheduler.metadata[j].stage = step; \
		m_Scheduler.worker_alerts[j]->notify_all(); \
	} m_Scheduler.lock.unlock();
	#define LAUNCH_UPDATE_STEP(step) m_Scheduler.lock.lock(); for (size_t j = 0; j < m_Scheduler.metadata.size(); j++) { \
		if (m_Scheduler.metadata[j].particles.index == 0 && m_Scheduler.metadata[j].particles.size == 0) continue; \
		m_Scheduler.metadata[j].finished = false; \
		m_Scheduler.metadata[j].stage = step; \
		m_Scheduler.worker_alerts[j]->notify_all(); \
	} m_Scheduler.lock.unlock();
	#define RESET_METADATA_TREES(trees) { \
		trees[0] = nullptr; \
		trees[1] = nullptr; \
		trees[2] = nullptr; \
		trees[3] = nullptr; \
		trees[4] = nullptr; \
		trees[5] = nullptr; \
		trees[6] = nullptr; \
		trees[7] = nullptr; \
	}

	// set up simulation progress and current particle slice
	std::vector<std::vector<Particle>> simulation_progress;
	m_ParticleSlice.clear();
	for (size_t i = 0; i < m_Particles.size(); i++) m_ParticleSlice.push_back(*m_Particles[i]);
	simulation_progress.push_back(m_ParticleSlice);

	// set up steps and unitsize
	uint64_t steps = m_SimulationLength / m_Timestep;
	if (m_UnitSize <= 0) m_UnitSize = EPS;

	// mutex lock and wait for all subprocesses to be ready
	while (true) {
		bool verified = true;
		for (size_t i = 0; i < m_Scheduler.metadata.size(); i++) {
			m_Scheduler.lock.lock();
			if (!m_Scheduler.metadata[i].finished) verified = false;
			m_Scheduler.lock.unlock();
			if (!verified) break;
		}
		if (verified) break;
	} 

	// simulate over a loop
	for (uint64_t i = 0; i < steps; i++) {
		if (m_Technique == SimulationTechnique::BARNESHUT) {
			// create enough of the octtree to paralellize
			double xdif = ((m_Scheduler.bounds.xmax - m_Scheduler.bounds.xmin) / 2.0);
			double ydif	= ((m_Scheduler.bounds.ymax - m_Scheduler.bounds.ymin) / 2.0);
			double zdif	= ((m_Scheduler.bounds.zmax - m_Scheduler.bounds.zmin) / 2.0);
			Oct space = {
				xdif + m_Scheduler.bounds.xmin,
				ydif + m_Scheduler.bounds.ymin,
				zdif + m_Scheduler.bounds.zmin,
				(xdif > ydif ? (xdif > zdif ? xdif : zdif) : (ydif > zdif ? ydif : zdif))
			};
			size_t treesize = 0;
			size_t ignoreind = 0;
			Octtree tree(space, &treesize);
			while (treesize < m_NumLocalWorkers) {
				tree.Insert(&m_ParticleSlice[ignoreind]);
				ignoreind++;
			}

			// parallelize the rest of the octtree creation
			std::vector<Octtree*> leaves;
			tree.GetLeaves(&leaves);
			m_Scheduler.lock.lock();
			for (size_t j = 0; j < leaves.size(); j++) {
				if (j == m_NumLocalWorkers - 1) {
					RESET_METADATA_TREES(m_Scheduler.metadata[j].trees);
					for (size_t k = j; k < leaves.size(); k++) {
						leaves[k]->m_SizeRef = nullptr;
						m_Scheduler.metadata[j].trees[k - j] = leaves[k];
					}
					m_Scheduler.metadata[j].ignore = ignoreind;
					m_Scheduler.metadata[j].finished = false;
					m_Scheduler.metadata[j].stage = WorkerStage::OCTTREE;
					m_Scheduler.worker_alerts[j]->notify_all();
					break;
				} else {
					RESET_METADATA_TREES(m_Scheduler.metadata[j].trees);
					leaves[j]->m_SizeRef = nullptr;
					m_Scheduler.metadata[j].trees[0] = leaves[j];
					m_Scheduler.metadata[j].ignore = ignoreind;
					m_Scheduler.metadata[j].finished = false;
					m_Scheduler.metadata[j].stage = WorkerStage::OCTTREE;
					m_Scheduler.worker_alerts[j]->notify_all();
				}
			}
			m_Scheduler.lock.unlock();
			WAIT_ON_WORKERS();

			// apply quadtree
			tree.CalculateCenterOfMass();
			m_Scheduler.lock.lock();
			for (size_t j = 0; j < m_Scheduler.metadata.size(); j++) {
				m_Scheduler.metadata[j].finished = false;
				m_Scheduler.metadata[j].stage = WorkerStage::APPLY;
				RESET_METADATA_TREES(m_Scheduler.metadata[j].trees);
				m_Scheduler.metadata[j].trees[0] = &tree;
				m_Scheduler.worker_alerts[j]->notify_all();
			}
			m_Scheduler.lock.unlock();
			WAIT_ON_WORKERS();
		} else if (m_Technique == SimulationTechnique::EDGE || m_Technique == SimulationTechnique::PARTICLE) {
			// launch the update acceleration step
			LAUNCH_NAIVE_STEP(WorkerStage::FORCEMATRIX);
			WAIT_ON_WORKERS();
		} else {
			FATAL("Unhandled technique");
		}

		// launch the update step to apply forces to particles
		LAUNCH_UPDATE_STEP(WorkerStage::UPDATE);
		WAIT_ON_WORKERS();

		// recalculate overall bounds
		m_Scheduler.bounds.Reset();
		for (size_t j = 0; j < m_NumLocalWorkers; j++) {
			BoundaryData b = m_Scheduler.metadata[j].bounds;
			if (b.xmin < m_Scheduler.bounds.xmin) m_Scheduler.bounds.xmin = b.xmin;
			if (b.ymin < m_Scheduler.bounds.ymin) m_Scheduler.bounds.ymin = b.ymin;
			if (b.zmin < m_Scheduler.bounds.zmin) m_Scheduler.bounds.zmin = b.zmin;
			if (b.xmax > m_Scheduler.bounds.xmax) m_Scheduler.bounds.xmax = b.xmax;
			if (b.ymax > m_Scheduler.bounds.ymax) m_Scheduler.bounds.ymax = b.ymax;
			if (b.zmax > m_Scheduler.bounds.zmax) m_Scheduler.bounds.zmax = b.zmax;
		}

		// update simulation progress
		simulation_progress.push_back(std::vector<Particle>(m_ParticleSlice));
		m_Scheduler.lock.lock();
		m_Progress = (float)((float)(i + 1) / (float)steps);
		m_Scheduler.lock.unlock();
	}

	// kill all subprocesses
	LAUNCH_NAIVE_STEP(WorkerStage::KILL);
	
	// notify done and finalize simulation record
    m_Scheduler.lock.lock();
	m_Finished = true;
	m_SimulationRecord = simulation_progress;
	m_Scheduler.lock.unlock();

	#undef WAIT_ON_WORKERS
	#undef LAUNCH_NAIVE_STEP
	#undef LAUNCH_UPDATE_STEP
	#undef RESET_METADATA_TREES
}

void Simulation::LocalJob(size_t index) {
	#define WAITJOB() { \
		std::unique_lock<std::mutex> lock(m_Scheduler.lock); \
		m_Scheduler.worker_alerts[index]->wait(lock, [this, &index] { return !m_Scheduler.metadata[index].finished; }); \
	}

	while (true) {
		WorkerStage stage = m_Scheduler.metadata[index].stage;
		switch (stage) {
			case WorkerStage::KILL:
				return;
			case WorkerStage::SETUP:
				break;
			case WorkerStage::FORCEMATRIX:
				if (m_Technique == SimulationTechnique::EDGE) {
					for (size_t i = 0; i < m_Scheduler.metadata[index].edges.size(); i++) {
						size_t r = m_Scheduler.metadata[index].edges[i].first;
						size_t c = m_Scheduler.metadata[index].edges[i].second;
						Particle px = m_ParticleSlice[r];
						Particle py = m_ParticleSlice[c];
						float dx = m_UnitSize * (py.Position().x - px.Position().x);
						float dy = m_UnitSize * (py.Position().y - px.Position().y);
						float dz = m_UnitSize * (py.Position().z - px.Position().z);
						float inv_r3 = std::pow((dx*dx) + (dy*dy) + (dz*dz) + (3*3), -1.5);
						glm::dvec3 pxa = { 0, 0, 0 };
						glm::dvec3 pya = { 0, 0, 0 };
						pxa.x = GRAVITY * (dx * inv_r3) * py.Mass();
						pxa.y = GRAVITY * (dy * inv_r3) * py.Mass();
						pxa.z = GRAVITY * (dz * inv_r3) * py.Mass();
						pya.x = -1.0 * pxa.x * px.Mass() / py.Mass();
						pya.y = -1.0 * pxa.y * px.Mass() / py.Mass();
						pya.z = -1.0 * pxa.z * px.Mass() / py.Mass();
						m_ForceMatrix[r][c] = pxa;
						m_ForceMatrix[c][r] = pya;
					}
				} else if (m_Technique == SimulationTechnique::PARTICLE) {
					for (size_t i = m_Scheduler.metadata[index].particles.index; i < m_Scheduler.metadata[index].particles.index + m_Scheduler.metadata[index].particles.size; i++) {
						for (size_t j = i + 1; j < m_Particles.size(); j++) {
							Particle px = m_ParticleSlice[i];
							Particle py = m_ParticleSlice[j];
							float dx = m_UnitSize * (py.Position().x - px.Position().x);
							float dy = m_UnitSize * (py.Position().y - px.Position().y);
							float dz = m_UnitSize * (py.Position().z - px.Position().z);
							float inv_r3 = std::pow((dx*dx) + (dy*dy) + (dz*dz) + (3*3), -1.5);
							glm::dvec3 pxa = { 0, 0, 0 };
							glm::dvec3 pya = { 0, 0, 0 };
							pxa.x = GRAVITY * (dx * inv_r3) * py.Mass();
							pxa.y = GRAVITY * (dy * inv_r3) * py.Mass();
							pxa.z = GRAVITY * (dz * inv_r3) * py.Mass();
							pya.x = -1.0 * pxa.x * px.Mass() / py.Mass();
							pya.y = -1.0 * pxa.y * px.Mass() / py.Mass();
							pya.z = -1.0 * pxa.z * px.Mass() / py.Mass();
							m_ForceMatrix[i][j] = pxa;
							m_ForceMatrix[j][i] = pya;
						}
					}
				} else {
					FATAL("Unhandled technique");
				}
				break;
			case WorkerStage::UPDATE:
				m_Scheduler.metadata[index].bounds.Reset();
				for (size_t i = m_Scheduler.metadata[index].particles.index; i < m_Scheduler.metadata[index].particles.index + m_Scheduler.metadata[index].particles.size; i++) {
					m_ParticleSlice[i].SetVelocity(m_ParticleSlice[i].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[i].Acceleration())/m_UnitSize);
					m_ParticleSlice[i].SetPosition(m_ParticleSlice[i].Position() + ((double)m_Timestep * m_ParticleSlice[i].Velocity()));
					if (m_Technique == SimulationTechnique::EDGE || 
						m_Technique == SimulationTechnique::PARTICLE) {
						glm::dvec3 accel = { 0.0, 0.0, 0.0 };
						for (size_t j = 0; j < m_Particles.size(); j++) {
							accel.x += m_ForceMatrix[i][j].x;
							accel.y += m_ForceMatrix[i][j].y;
							accel.z += m_ForceMatrix[i][j].z;
						}
						m_ParticleSlice[i].SetAcceleration(accel);
					}
					m_ParticleSlice[i].SetVelocity(m_ParticleSlice[i].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[i].Acceleration())/m_UnitSize);
					glm::dvec3 pos = m_ParticleSlice[i].Position();
					if (pos.x < m_Scheduler.metadata[index].bounds.xmin) m_Scheduler.metadata[index].bounds.xmin = pos.x - 0.001;
					if (pos.y < m_Scheduler.metadata[index].bounds.ymin) m_Scheduler.metadata[index].bounds.ymin = pos.y - 0.001;
					if (pos.z < m_Scheduler.metadata[index].bounds.zmin) m_Scheduler.metadata[index].bounds.zmin = pos.z - 0.001;
					if (pos.x > m_Scheduler.metadata[index].bounds.xmax) m_Scheduler.metadata[index].bounds.xmax = pos.x + 0.001;
					if (pos.y > m_Scheduler.metadata[index].bounds.ymax) m_Scheduler.metadata[index].bounds.ymax = pos.y + 0.001;
					if (pos.z > m_Scheduler.metadata[index].bounds.zmax) m_Scheduler.metadata[index].bounds.zmax = pos.z + 0.001;
				}
				break;
			case WorkerStage::OCTTREE:
				for (size_t i = 0; i < 8; i++) {
					Octtree* tree = m_Scheduler.metadata[index].trees[i];
					if (tree != nullptr) {
						for (size_t j = m_Scheduler.metadata[index].ignore; j < m_Particles.size(); j++) {
							if (tree->m_Boundary.Contains(&m_ParticleSlice[j])) tree->Insert(&m_ParticleSlice[j]);
						}
					} else {
						break;
					}
				}
				break;
			case WorkerStage::APPLY:
				for (size_t i = m_Scheduler.metadata[index].particles.index; i < m_Scheduler.metadata[index].particles.index + m_Scheduler.metadata[index].particles.size; i++) {
					m_ParticleSlice[i].SetAcceleration({0.0, 0.0, 0.0});
					m_Scheduler.metadata[index].trees[0]->SerialCalculateForce(m_ParticleSlice[i], m_UnitSize);
				}
				break;
			default:
				FATAL("Unknown worker stage");
				break;
		}
		// lock, update, and wait to continue
		m_Scheduler.lock.lock();
		m_Scheduler.metadata[index].finished = true;
		m_Scheduler.controller_alert.notify_all();
		m_Scheduler.lock.unlock();
		WAITJOB();
	}

	#undef WAITJOB
}

void Simulation::Host() {
	ResetClients();
	m_Network = CreateRef<Network>(this);
	grpc::ServerBuilder builder;
	builder.AddListeningPort(m_HostAddress, grpc::InsecureServerCredentials());
	builder.RegisterService(&(*m_Network));
	Scope<grpc::Server> server = builder.BuildAndStart();
	m_ServerData.running = true;
}

void Simulation::ResetClients() {
	m_ServerData.num_clients = 0;
	m_Clients.clear();
	for (size_t i = 0; i < m_NumRemoteWorkers; i++) {
		ClientMetadata data;
		m_Clients.push_back(data);
	}
}

bool Simulation::RegisterClient(std::string& ipaddr) {
	if (m_ServerData.num_clients >= m_Clients.size()) return false;
	m_Clients[m_ServerData.num_clients].connected = true;
	m_Clients[m_ServerData.num_clients].ip = ipaddr;
	return true;
}

void Simulation::Start() {
    this->Log("starting simulation...");

	// clear any lingering subprocesses
	m_SubProcesses.clear();

	// prep force matrix
	m_ForceMatrix.resize(m_Particles.size(), std::vector<glm::dvec3>(m_Particles.size(), {0, 0, 0}));

	// reset and calculate initial bounds
	m_Scheduler.bounds.Reset();
	for (size_t i = 0; i < m_Particles.size(); i++) {
		glm::dvec3 pos = m_Particles[i]->Position();
		if (pos.x < m_Scheduler.bounds.xmin) m_Scheduler.bounds.xmin = pos.x - 0.001;
		if (pos.y < m_Scheduler.bounds.ymin) m_Scheduler.bounds.ymin = pos.y - 0.001;
		if (pos.z < m_Scheduler.bounds.zmin) m_Scheduler.bounds.zmin = pos.z - 0.001;
		if (pos.x > m_Scheduler.bounds.xmax) m_Scheduler.bounds.xmax = pos.x + 0.001;
		if (pos.y > m_Scheduler.bounds.ymax) m_Scheduler.bounds.ymax = pos.y + 0.001;
		if (pos.z > m_Scheduler.bounds.zmax) m_Scheduler.bounds.zmax = pos.z + 0.001;
	}

	if (m_Technique == SimulationTechnique::PARTICLE || m_Technique == SimulationTechnique::BARNESHUT) {
		// optimize the number of workers for particles
		if (m_NumLocalWorkers > m_Particles.size()) {
			m_NumLocalWorkers = m_Particles.size();
			this->Log("more workers than possible jobs detected. truncating extra workers...");
		}
		bool uneven = m_Particles.size() % m_NumLocalWorkers != 0;
		size_t jobsize = uneven ? (m_Particles.size() / m_NumLocalWorkers) + 1 : m_Particles.size() / m_NumLocalWorkers;
		size_t optimal_workers = m_Particles.size() % jobsize != 0 ? (m_Particles.size() / jobsize) + 1 : m_Particles.size() / jobsize;
		if (optimal_workers != m_NumLocalWorkers) {
			this->Log("more workers than possible jobs needed. truncating additional workers...");
			m_NumLocalWorkers = optimal_workers;
		}

		// set up scheduler
		m_Scheduler.metadata.clear();
		m_Scheduler.worker_alerts.clear();
		for (uint32_t i = 0; i < m_NumLocalWorkers; i++) {
			ParticleJobData data = { i * jobsize, jobsize };
			if (i == m_NumLocalWorkers - 1 && uneven) data.size = m_Particles.size() % jobsize;
			m_Scheduler.metadata.push_back({
				WorkerStage::SETUP,
				true,
				false,
				data,
				{},
				{},
				0
			});
			m_Scheduler.worker_alerts.push_back(CreateScope<std::condition_variable>());
		}
	} else {
		// calculate number of edges and optimal jobsize
		size_t numedges = ((m_Particles.size() * m_Particles.size()) - m_Particles.size()) / 2;
		if (m_NumLocalWorkers > numedges) {
			m_NumLocalWorkers = numedges;
			this->Log("more workers than possible jobs detected. truncating extra workers...");
		}
		bool uneven = numedges % m_NumLocalWorkers != 0;
		size_t jobsize = uneven ? (numedges / m_NumLocalWorkers) + 1 : numedges / m_NumLocalWorkers;
		size_t optimal_workers = numedges % jobsize != 0 ? (numedges / jobsize) + 1 : numedges / jobsize;
		if (optimal_workers != m_NumLocalWorkers) {
			this->Log("more workers than possible jobs needed. truncating additional workers...");
			m_NumLocalWorkers = optimal_workers;
		}
		size_t second_jobsize = m_Particles.size() % optimal_workers != 0 ? (m_Particles.size() / optimal_workers) + 1 : m_Particles.size() / optimal_workers;
	
		// set up scheduler
		m_Scheduler.metadata.clear();
		m_Scheduler.worker_alerts.clear();
		m_Scheduler.metadata.push_back({
			WorkerStage::SETUP,
			true,
			false,
			{ 0, 0 },
			{},
			{},
			0
		});
		m_Scheduler.worker_alerts.push_back(CreateScope<std::condition_variable>());
		size_t p_ind = 0;
		for (size_t r = 0; r < m_Particles.size(); r++) {
			for (size_t c = 0; c < m_Particles.size(); c++) {
				if (c <= r) c = r + 1;
				if (c >= m_Particles.size()) continue;
				if (m_Scheduler.metadata[m_Scheduler.metadata.size() - 1].edges.size() >= jobsize) {
					size_t range = second_jobsize;
					size_t ind = p_ind;
					if (p_ind + range > m_Particles.size()) range = m_Particles.size() - p_ind;
					if (p_ind >= m_Particles.size()) {
						ind = 0;
						range = 0;
					}
					m_Scheduler.metadata[m_Scheduler.metadata.size() - 1].particles = { ind, range };
					p_ind += range;
					m_Scheduler.metadata.push_back({
						WorkerStage::SETUP,
						true,
						false,
						{ 0, 0 },
						{},
						{},
						0
					});
					m_Scheduler.worker_alerts.push_back(CreateScope<std::condition_variable>());
				}
				m_Scheduler.metadata[m_Scheduler.metadata.size() - 1].edges.push_back(std::pair<size_t, size_t>(r, c));
			}
		}
		if (m_Scheduler.metadata[m_Scheduler.metadata.size() - 1].edges.size() > 0) {
			size_t range = second_jobsize;
			size_t ind = p_ind;
			if (p_ind + range > m_Particles.size()) range = m_Particles.size() - p_ind;
			if (p_ind >= m_Particles.size()) {
				ind = 0;
				range = 0;
			}
			m_Scheduler.metadata[m_Scheduler.metadata.size() - 1].particles = { ind, range };
		}

		// verify creation
		if (m_NumLocalWorkers != m_Scheduler.metadata.size()) FATAL("Invalid job creation detected");
	}

	// create subprocesses
	for (uint32_t i = 0; i < m_NumLocalWorkers; i++) {
		m_SubProcesses.push_back(std::thread(&Simulation::LocalJob, this, i));
	}

	// create main process
	m_MainProcess = std::thread(&Simulation::Simulate, this);
	
	// start tracking
    m_Started = true;
	m_Paused = false;
	m_TimeTrack = TIMENOW();
}

void Simulation::Pause() {
    this->Log("pausing simulation...");
	m_Scheduler.controller_alert.notify_all();
    this->Log("paused simulation");
    m_Paused = true; 
}

void Simulation::Resume() {
    this->Log("resuming simulation...");
    m_Paused = false; 
}

void Simulation::Abort() {
    this->Log("aborting simulation...");

    this->Log("aborted simulation");
    m_Started = false; 
    m_Paused = false; 
}

void Simulation::Prime() {   
	m_Progress = 0.0f;
	m_Started = false;
	m_Paused = false;
	m_Finished = false;
}

void Simulation::Checkup() {
	if (m_Finished) {
		m_MainProcess.join();
		for (size_t i = 0; i < m_SubProcesses.size(); i++) {
			m_SubProcesses[i].join();
		}
		m_TimeTrack = TIMENOW() - m_TimeTrack;
		m_Started = false;
		m_Paused = false;
    	std::stringstream logstream;
    	logstream << "finished simulation in " 
               << std::setw(2) << std::setfill('0') << (m_TimeTrack / 3600000) << ':'
               << std::setw(2) << std::setfill('0') << ((m_TimeTrack / 60000) % 60) << ':'
               << std::setw(2) << std::setfill('0') << ((m_TimeTrack / 1000) % 60) << ':'
			   << std::setw(3) << std::setfill('0') << (m_TimeTrack % 1000);
		this->Log(logstream.str());
	}
}
