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
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&currentTime);
    std::stringstream timeStream;
    timeStream << '[' 
               << std::setw(2) << std::setfill('0') << localTime.tm_hour << ':'
               << std::setw(2) << std::setfill('0') << localTime.tm_min << ':'
               << std::setw(2) << std::setfill('0') << localTime.tm_sec << ']';
    return timeStream.str();
}

void Simulation::Simulate() {
	std::vector<std::vector<glm::vec2>> force_matrix(m_Particles.size(), std::vector<glm::vec2>(m_Particles.size(), {0, 0}));
	std::vector<std::vector<Particle>> simulation_progress;
	std::vector<Particle> particle_slice;
	for (size_t i = 0; i < m_Particles.size(); i++) particle_slice.push_back(*m_Particles[i]);

	uint64_t steps = m_SimulationLength / m_Timestep;
	simulation_progress.push_back(particle_slice);

	for (uint64_t i = 0; i < steps; i++) {
        for (size_t j = 0; j < particle_slice.size(); j++) {
            particle_slice[j].SetPosition(particle_slice[j].Position() + particle_slice[j].Velocity());
            particle_slice[j].SetVelocity(particle_slice[j].Velocity() + (0.5f * m_Timestep * particle_slice[j].Acceleration()));
        }

		for (size_t j = 0; j < particle_slice.size(); j++) {
			particle_slice[j].SetAcceleration({ 0, 0, 0 });
		}

		for (size_t j = 0; j < particle_slice.size(); j++) {
			for (size_t k = 0; k < particle_slice.size(); k++) {
				if (j != k) {
					Particle px = particle_slice[j];
					Particle py = particle_slice[k];
					float dx = py.Position().x - px.Position().x;
					float dy = py.Position().y - px.Position().y;
					float inv_r3 = std::pow((dx*dx) + (dy*dy) + (3*3), -1.5);
					glm::vec3 pxa = { 0, 0, 0 };
					glm::vec3 pya = { 0, 0, 0 };
					pxa.x = G * (dx * inv_r3) * py.Mass();
					pxa.y = G * (dy * inv_r3) * py.Mass();
					pya.x = -1.0 * pxa.x * px.Mass() / py.Mass();
					pya.y = -1.0 * pxa.y * px.Mass() / py.Mass();
					particle_slice[j].SetAcceleration(px.Acceleration() + pxa);
					particle_slice[j].SetAcceleration(py.Acceleration() + pya);
				}
			}
		}

        for (size_t j = 0; j < particle_slice.size(); j++) {
            particle_slice[j].SetVelocity(particle_slice[j].Velocity() + (0.5f * m_Timestep * particle_slice[j].Acceleration()));
        }

		// TODO: make copying the current slice multi-threaded so its non-blocking
		simulation_progress.push_back(std::vector<Particle>(particle_slice));
		m_MutexLock.lock();
		m_Progress = (float)((float)(i + 1) / (float)steps);
		m_MutexLock.unlock();
	}

    m_MutexLock.lock();
	m_Finished = true;
	m_MutexLock.unlock();
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
    m_MainProcess = std::thread(&Simulation::Simulate, this);
    m_Started = true; m_Paused = false; 
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

void Simulation::Checkup() {
	if (m_Finished) {
		m_MainProcess.join();
		m_Started = false;
		m_Paused = false;
		this->Log("finished simulation!");
	}
}
