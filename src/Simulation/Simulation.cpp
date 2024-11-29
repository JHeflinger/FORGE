#include "Simulation.h"
#include "Core/Log.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <cmath>

#define EPS 0.0000000000001 // epsilon for numerical stability
#define G 0.0000000000667430

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

void Simulation::Simulate() {
	std::vector<std::vector<Particle>> simulation_progress;
	m_ParticleSlice.clear();
	for (size_t i = 0; i < m_Particles.size(); i++) m_ParticleSlice.push_back(*m_Particles[i]);

	uint64_t steps = m_SimulationLength / m_Timestep;
	simulation_progress.push_back(m_ParticleSlice);

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

void Simulation::ParticleJob(size_t index, size_t range) {
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
		for (size_t j = index; j < index + range; j++) {
			m_ParticleSlice[j].SetPosition(m_ParticleSlice[j].Position() + ((double)m_Timestep * m_ParticleSlice[j].Velocity()));
			m_ParticleSlice[j].SetVelocity(m_ParticleSlice[j].Velocity() + (0.5 * m_Timestep * m_ParticleSlice[j].Acceleration())/m_UnitSize);
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
		for (size_t j = index; j < index + range; j++) {
			for (size_t k = j + 1; k < m_Particles.size(); k++) {
				Particle px = m_ParticleSlice[j];
				Particle py = m_ParticleSlice[k];
				float dx = m_UnitSize * (py.Position().x - px.Position().x);
				float dy = m_UnitSize * (py.Position().y - px.Position().y);
				float inv_r3 = std::pow((dx*dx) + (dy*dy) + (3*3), -1.5);
				glm::dvec3 pxa = { 0, 0, 0 };
				glm::dvec3 pya = { 0, 0, 0 };
				pxa.x = G * (dx * inv_r3) * py.Mass();
				pxa.y = G * (dy * inv_r3) * py.Mass();
				pya.x = -1.0 * pxa.x * px.Mass() / py.Mass();
				pya.y = -1.0 * pxa.y * px.Mass() / py.Mass();
				m_ForceMatrix[j][k] = pxa;
				m_ForceMatrix[k][j] = pya;
			}
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

void Simulation::Start() {
    this->Log("starting simulation...");
	m_FinishedWorkers.push_back(0);
	m_FinishedWorkers.push_back(0);
	m_FinishedWorkers.push_back(0);
	m_ForceMatrix.resize(m_Particles.size(), std::vector<glm::dvec3>(m_Particles.size(), {0, 0, 0}));
    m_MainProcess = std::thread(&Simulation::Simulate, this);
	m_SubProcesses.clear();
	if (m_NumLocalWorkers > m_Particles.size()) {
		m_NumLocalWorkers = m_Particles.size();
		this->Log("more workers than possible jobs detected. truncating extra workers...");
	}
	bool uneven = m_Particles.size() % m_NumLocalWorkers != 0;
	size_t jobsize = uneven ? (m_Particles.size() / m_NumLocalWorkers) + 1 : m_Particles.size() / m_NumLocalWorkers;
	for (int i = 0; i < m_NumLocalWorkers; i++) {
		if (i == m_NumLocalWorkers - 1 && uneven) {
			m_SubProcesses.push_back(std::thread(&Simulation::ParticleJob, this, i * jobsize, (m_Particles.size() % jobsize)));
		} else {
			m_SubProcesses.push_back(std::thread(&Simulation::ParticleJob, this, i * jobsize, jobsize));
		}
	}
    m_Started = true;
	m_Paused = false;
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
		for (int i = 0; i < m_NumLocalWorkers; i++) {
			m_SubProcesses[i].join();
		}
		m_Started = false;
		m_Paused = false;
		this->Log("finished simulation!");
	}
}