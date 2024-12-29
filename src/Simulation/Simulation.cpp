#include "Simulation.h"
#include "Utils/Quadtree.h"
#include "Core/Log.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <cmath>

#define EPS 0.0000000000001 // epsilon for numerical stability
#ifndef G
#define G 0.0000000000667430
#endif
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

void Simulation::AccurateSimulate() {
	std::vector<std::vector<Particle>> simulation_progress;
	m_ParticleSlice.clear();
	for (size_t i = 0; i < m_Particles.size(); i++) m_ParticleSlice.push_back(*m_Particles[i]);
	simulation_progress.push_back(m_ParticleSlice);

	uint64_t steps = m_SimulationLength / m_Timestep;
	if (m_UnitSize <= 0) m_UnitSize = EPS;

	// wait for local workers to be ready
	{
		std::unique_lock<std::mutex> lock(m_MutexLock);
		m_ControllerAlert.wait(lock, [this] { return m_FinishedWorkers[0] >= m_NumLocalWorkers; });
	}

	for (uint64_t i = 0; i < steps; i++) {
		// alert local workers to update position and velocity
		m_MutexLock.lock();
		m_FinishedWorkers[0] = 0;
		m_MutexLock.unlock();
		m_WorkerAlert.notify_all();

		// wait on local workers to finish
		{
			std::unique_lock<std::mutex> lock(m_MutexLock);
			m_ControllerAlert.wait(lock, [this] { return m_FinishedWorkers[1] >= m_NumLocalWorkers; });
		}

		// alert local workers to update accleration
		m_MutexLock.lock();
		m_FinishedWorkers[1] = 0;
		m_MutexLock.unlock();
		m_WorkerAlert.notify_all();

		// wait on local workers to finish
		{
			std::unique_lock<std::mutex> lock(m_MutexLock);
			m_ControllerAlert.wait(lock, [this] { return m_FinishedWorkers[2] >= m_NumLocalWorkers; });
		}

		// alert local workers to update velocity
		m_MutexLock.lock();
		m_FinishedWorkers[2] = 0;
		m_MutexLock.unlock();
		m_WorkerAlert.notify_all();

		// wait on local workers to finish
		{
			std::unique_lock<std::mutex> lock(m_MutexLock);
			m_ControllerAlert.wait(lock, [this] { return m_FinishedWorkers[0] >= m_NumLocalWorkers; });
		}

		// copy and push the slice
		// TODO: make copying the current slice multi-threaded so its non-blocking
		simulation_progress.push_back(std::vector<Particle>(m_ParticleSlice));
		m_MutexLock.lock();
		m_Progress = (float)((float)(i + 1) / (float)steps);
		m_MutexLock.unlock();
	}
    m_MutexLock.lock();
	m_Finished = true;
	m_SimulationRecord = simulation_progress;
	m_FinishedWorkers[0] = 0;
	m_MutexLock.unlock();
	m_WorkerAlert.notify_all();
}

void Simulation::ApproximateSimulate() {
	std::vector<std::vector<Particle>> simulation_progress;
	m_ParticleSlice.clear();
	for (size_t i = 0; i < m_Particles.size(); i++) m_ParticleSlice.push_back(*m_Particles[i]);
	simulation_progress.push_back(m_ParticleSlice);

	uint64_t steps = m_SimulationLength / m_Timestep;
	if (m_UnitSize <= 0) m_UnitSize = EPS;

	for (uint64_t i = 0; i < steps; i++) {
		// update velocity and position
		for (size_t j = 0; j < m_Particles.size(); j++) {
			m_ParticleSlice[j].SetPosition(m_ParticleSlice[j].Position() + ((double)m_Timestep * m_ParticleSlice[j].Velocity()));
			m_ParticleSlice[j].SetVelocity(m_ParticleSlice[j].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[j].Acceleration())/m_UnitSize);
		}

		// reset accleration
		for (size_t j = 0; j < m_Particles.size(); j++) {
			m_ParticleSlice[j].SetAcceleration({0.0, 0.0, 0.0});
		}

		// create quadtree
		Quad space = { 0, 0, 100.0 };
		Quadtree tree(space);
		for (size_t j = 0; j < m_Particles.size(); j++) {
			tree.Insert(&(*m_Particles[j]));
		}

		// use quadtree to calculate acceleration
		tree.CalculateCenterOfMass();
		for (size_t j = 0; j < m_Particles.size(); j++) {
			tree.SerialCalculateForce(m_ParticleSlice[j], m_UnitSize);
		}

		// update velocity
		for (size_t j = 0; j < m_Particles.size(); j++) {
			m_ParticleSlice[j].SetVelocity(m_ParticleSlice[j].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[j].Acceleration())/m_UnitSize);
		}
		
		simulation_progress.push_back(std::vector<Particle>(m_ParticleSlice));
		m_MutexLock.lock();
		m_Progress = (float)((float)(i + 1) / (float)steps);
		m_MutexLock.unlock();
	}
    m_MutexLock.lock();
	m_Finished = true;
	m_SimulationRecord = simulation_progress;
	m_MutexLock.unlock();
}

void Simulation::EdgeJob(std::vector<std::pair<size_t, size_t>> edges, size_t index, size_t range) {
	m_MutexLock.lock();
	m_FinishedWorkers[0]++;
	m_MutexLock.unlock();
	m_ControllerAlert.notify_all();

	{
		std::unique_lock<std::mutex> lock(m_MutexLock);
		m_WorkerAlert.wait(lock, [this] { return m_FinishedWorkers[0] == 0; });
	}

	uint64_t steps = m_SimulationLength / m_Timestep;
	for (uint64_t i = 0; i < steps; i++) {
		// update velocity and position
		if (index != 0 || range != 0) {
			for (size_t j = index; j < index + range; j++) {
				m_ParticleSlice[j].SetPosition(m_ParticleSlice[j].Position() + ((double)m_Timestep * m_ParticleSlice[j].Velocity()));
				m_ParticleSlice[j].SetVelocity(m_ParticleSlice[j].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[j].Acceleration())/m_UnitSize);
			}
		}
		m_MutexLock.lock();
		m_FinishedWorkers[1]++;
		m_MutexLock.unlock();
		m_ControllerAlert.notify_all();
		{
			std::unique_lock<std::mutex> lock(m_MutexLock);
			m_WorkerAlert.wait(lock, [this] { return m_FinishedWorkers[1] == 0; });
		}

		// update acceleration
		for (size_t j = 0; j < edges.size(); j++) {
			size_t r = edges[j].first;
			size_t c = edges[j].second;
			Particle px = m_ParticleSlice[r];
			Particle py = m_ParticleSlice[c];
			float dx = m_UnitSize * (py.Position().x - px.Position().x);
			float dy = m_UnitSize * (py.Position().y - px.Position().y);
			float dz = m_UnitSize * (py.Position().z - px.Position().z);
			float inv_r3 = std::pow((dx*dx) + (dy*dy) + (dz*dz) + (3*3), -1.5);
			glm::dvec3 pxa = { 0, 0, 0 };
			glm::dvec3 pya = { 0, 0, 0 };
			pxa.x = G * (dx * inv_r3) * py.Mass();
			pxa.y = G * (dy * inv_r3) * py.Mass();
			pxa.z = G * (dz * inv_r3) * py.Mass();
			pya.x = -1.0 * pxa.x * px.Mass() / py.Mass();
			pya.y = -1.0 * pxa.y * px.Mass() / py.Mass();
			pya.z = -1.0 * pxa.z * px.Mass() / py.Mass();
			m_ForceMatrix[r][c] = pxa;
			m_ForceMatrix[c][r] = pya;
		}
		m_MutexLock.lock();
		m_FinishedWorkers[2]++;
		m_MutexLock.unlock();
		m_ControllerAlert.notify_all();
		{
			std::unique_lock<std::mutex> lock(m_MutexLock);
			m_WorkerAlert.wait(lock, [this] { return m_FinishedWorkers[2] == 0; });
		}

		// update velocity
		if (index != 0 || range != 0) {
			for (size_t j = index; j < index + range; j++) {
				glm::dvec3 accel = { 0.0, 0.0, 0.0 };
				for (size_t k = 0; k < m_Particles.size(); k++) {
					accel.x += m_ForceMatrix[j][k].x;
					accel.y += m_ForceMatrix[j][k].y;
					accel.z += m_ForceMatrix[j][k].z;
				}
				m_ParticleSlice[j].SetAcceleration(accel);
				m_ParticleSlice[j].SetVelocity(m_ParticleSlice[j].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[j].Acceleration())/m_UnitSize);
			}
		}
		m_MutexLock.lock();
		m_FinishedWorkers[0]++;
		m_MutexLock.unlock();
		m_ControllerAlert.notify_all();
		{
			std::unique_lock<std::mutex> lock(m_MutexLock);
			m_WorkerAlert.wait(lock, [this] { return m_FinishedWorkers[0] == 0; });
		}
	}
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
	#define LAUNCH_NAIVE_STEP(step)	for (size_t j = 0; j < m_Scheduler.metadata.size(); j++) { \
		m_Scheduler.metadata[j].finished = false; \
		m_Scheduler.metadata[j].stage = step; \
		m_Scheduler.worker_alerts[j]->notify_all(); \
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
		// launch the step to update velocity and position by half
		LAUNCH_NAIVE_STEP(WorkerStage::VP_HALFSTEP);
		WAIT_ON_WORKERS();
		
		if (m_Technique == SimulationTechnique::BARNESHUT) {
			// TODO:
			// all of this
		} else if (m_Technique == SimulationTechnique::EDGE || m_Technique == SimulationTechnique::PARTICLE) {
			// launch the update acceleration step
			LAUNCH_NAIVE_STEP(WorkerStage::A_STEP);
			WAIT_ON_WORKERS();
		} else {
			FATAL("Unhandled technique");
		}

		// launch the update velocity by half step
		LAUNCH_NAIVE_STEP(WorkerStage::V_HALFSTEP);
		WAIT_ON_WORKERS();

		// update simulation progress
		simulation_progress.push_back(std::vector<Particle>(m_ParticleSlice));
		m_MutexLock.lock();
		m_Progress = (float)((float)(i + 1) / (float)steps);
		m_MutexLock.unlock();
	}

	// kill all subprocesses
	LAUNCH_NAIVE_STEP(WorkerStage::KILL);
	
	// notify done and finalize simulation record
    m_MutexLock.lock();
	m_Finished = true;
	m_SimulationRecord = simulation_progress;
	m_MutexLock.unlock();

	#undef WAIT_ON_WORKERS
	#undef LAUNCH_NAIVE_STEP
}

void Simulation::WorkerJob(size_t index) {
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
			case WorkerStage::VP_HALFSTEP:
				for (size_t i = m_Scheduler.metadata[index].particles.index; i < m_Scheduler.metadata[index].particles.index + m_Scheduler.metadata[index].particles.size; i++) {
					m_ParticleSlice[i].SetPosition(m_ParticleSlice[i].Position() + ((double)m_Timestep * m_ParticleSlice[i].Velocity()));
					m_ParticleSlice[i].SetVelocity(m_ParticleSlice[i].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[i].Acceleration())/m_UnitSize);
				}
				break;
			case WorkerStage::V_HALFSTEP:
				for (size_t i = m_Scheduler.metadata[index].particles.index; i < m_Scheduler.metadata[index].particles.index + m_Scheduler.metadata[index].particles.size; i++) {
					if (m_Scheduler.metadata[index].type == SimulationTechnique::EDGE || 
						m_Scheduler.metadata[index].type == SimulationTechnique::PARTICLE) {
						glm::dvec3 accel = { 0.0, 0.0, 0.0 };
						for (size_t j = 0; j < m_Particles.size(); j++) {
							accel.x += m_ForceMatrix[i][j].x;
							accel.y += m_ForceMatrix[i][j].y;
							accel.z += m_ForceMatrix[i][j].z;
						}
						m_ParticleSlice[i].SetAcceleration(accel);
					}
					m_ParticleSlice[i].SetVelocity(m_ParticleSlice[i].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[i].Acceleration())/m_UnitSize);
				}
				break;
			case WorkerStage::A_STEP:
				if (m_Scheduler.metadata[index].type == SimulationTechnique::BARNESHUT) {
					// TODO:
				} else if (m_Scheduler.metadata[index].type == SimulationTechnique::EDGE) {
					// TODO:
				} else if (m_Scheduler.metadata[index].type == SimulationTechnique::PARTICLE) {
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
							pxa.x = G * (dx * inv_r3) * py.Mass();
							pxa.y = G * (dy * inv_r3) * py.Mass();
							pxa.z = G * (dz * inv_r3) * py.Mass();
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
			default:
				FATAL("Unknown worker stage");
				break;
		}

		// lock, update, and wait to continue
		m_Scheduler.lock.lock();
		m_Scheduler.metadata[index].finished = true;
		m_Scheduler.lock.unlock();
		m_Scheduler.controller_alert.notify_all();
		WAITJOB();
	}

	#undef WAITJOB
}

void Simulation::Start() {
    this->Log("starting simulation...");
	if (m_Technique == SimulationTechnique::PARTICLE) {
		// clear any lingering subprocesses
		m_SubProcesses.clear();

		// prep force matrix
		m_ForceMatrix.resize(m_Particles.size(), std::vector<glm::dvec3>(m_Particles.size(), {0, 0, 0}));

		// optimize the number of workers
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
				SimulationTechnique::PARTICLE,
				true,
				false,
				data
			});
			m_Scheduler.worker_alerts.push_back(CreateScope<std::condition_variable>());
		}

		// create subprocesses
		for (uint32_t i = 0; i < m_NumLocalWorkers; i++) {
			m_SubProcesses.push_back(std::thread(&Simulation::WorkerJob, this, i));
		}

		// create main process
		m_MainProcess = std::thread(&Simulation::Simulate, this);
	} else if (m_Technique == SimulationTechnique::EDGE) {
		m_MainProcess = std::thread(&Simulation::AccurateSimulate, this);
		m_SubProcesses.clear();
		m_FinishedWorkers.push_back(0);
		m_FinishedWorkers.push_back(0);
		m_FinishedWorkers.push_back(0);
		m_ForceMatrix.resize(m_Particles.size(), std::vector<glm::dvec3>(m_Particles.size(), {0, 0, 0}));
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
		size_t p_ind = 0;
		std::vector<std::pair<size_t, size_t>> edges;
		for (size_t r = 0; r < m_Particles.size(); r++) {
			for (size_t c = 0; c < m_Particles.size(); c++) {
				if (c <= r) c = r + 1;
				if (c >= m_Particles.size()) continue;
				if (edges.size() >= jobsize) {
					size_t range = second_jobsize;
					size_t ind = p_ind;
					if (p_ind + range > m_Particles.size()) range = m_Particles.size() - p_ind;
					if (p_ind >= m_Particles.size()) {
						ind = 0;
						range = 0;
					}
					m_SubProcesses.push_back(std::thread(&Simulation::EdgeJob, this, edges, ind, range));
					p_ind += range;
					edges.clear();
				}
				edges.push_back(std::pair<size_t, size_t>(r, c));
			}
		}
		if (edges.size() > 0) {
			size_t range = second_jobsize;
			size_t ind = p_ind;
			if (p_ind + range > m_Particles.size()) range = m_Particles.size() - p_ind;
			if (p_ind >= m_Particles.size()) {
				ind = 0;
				range = 0;
			}
			m_SubProcesses.push_back(std::thread(&Simulation::EdgeJob, this, edges, ind, range));
		}
	} else if (m_Technique == SimulationTechnique::BARNESHUT) {
		m_MainProcess = std::thread(&Simulation::ApproximateSimulate, this);
		m_SubProcesses.clear();
	} else {
		this->Log("Technique selected has not been implemented. Unable to start simulation.");
		return;
	}
    m_Started = true;
	m_Paused = false;
	m_TimeTrack = TIMENOW();
}

void Simulation::Pause() {
    this->Log("pausing simulation...");

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
