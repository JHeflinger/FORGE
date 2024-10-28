#pragma once
#include "Simulation/Grid.h"
#include "Simulation/Particle.h"
#include "Simulation/Sink.h"
#include "Simulation/Source.h"
#include "Core/Safety.h"
#include "glm/glm.hpp"
#include <vector>
#include <mutex>
#include <thread>

enum class SimulationLengthUnit {
	TICKS = 0,
	MICROSECONDS = 1,
	MILLISECONDS = 2,
	SECONDS = 3,
};

enum class SimulationSolver {
	RKF45 = 0,
	EULER = 1,
	LEAPFROG = 2,
};

class Simulation {
public:
    std::vector<Ref<Source>>& Sources() { return m_Sources; }
    std::vector<Ref<Sink>>& Sinks() { return m_Sinks; }
    std::vector<Ref<Particle>>& Particles() { return m_Particles; }
    std::vector<Ref<Grid>>& Grids() { return m_Grids; }
public:
    std::string Filepath() { return m_Filepath; }
    void SetFilepath(std::string path) { m_Filepath = path; }
	SimulationLengthUnit LengthUnit() { return m_LengthUnit; }
	void SetLengthUnit(SimulationLengthUnit unit) { m_LengthUnit = unit; }
	uint64_t Length() { return m_SimulationLength; }
	void SetLength(uint64_t length) { m_SimulationLength = length; }
	bool SafeguardCacheEnabled() { return m_EnableSafeguardCache; }
	void SetSafeguardCache(bool enabled) { m_EnableSafeguardCache = enabled; }
	bool SimulationRecordEnabled() { return m_EnableSimulationRecord; }
	void SetSimulationRecord(bool enabled) { m_EnableSimulationRecord = enabled; }
	SimulationSolver Solver() { return m_Solver; }
	void SetSolver(SimulationSolver solver) { m_Solver = solver; }
	glm::dvec2 Bounds() { return m_Bounds; }
	void SetBounds(glm::vec2 bounds) { m_Bounds = bounds; } 
	uint64_t Timestep() { return m_Timestep; }
	void SetTimestep(uint64_t ts) { m_Timestep = ts; }
	bool DynamicTimestep() { return m_DynamicTimestep; }
	void SetDynamicTimestep(bool dynamic) { m_DynamicTimestep = dynamic; }
	uint32_t NumLocalWorkers() { return m_NumLocalWorkers; }
	void SetNumLocalWorkers(uint32_t workers) { m_NumLocalWorkers = workers; }
	uint32_t NumRemoteWorkers() { return m_NumRemoteWorkers; }
	void SetNumRemoteWorkers(uint32_t workers) { m_NumRemoteWorkers = workers; }
public:
	void Log(std::string log);
	std::vector<std::string>& Logs() { return m_Logs; }
	float Progress() { return m_Progress; }
	bool Finished() { return m_Finished; }
	bool Started();
	bool Paused();
	void Start();
	void Pause();
	void Resume();
	void Abort();
	void Checkup();
	void Simulate();
	void Prime();
	std::vector<std::vector<Particle>>& SimulationRecord() { return m_SimulationRecord; }
private:
    std::vector<Ref<Source>> m_Sources;
    std::vector<Ref<Sink>> m_Sinks;
    std::vector<Ref<Particle>> m_Particles;
    std::vector<Ref<Grid>> m_Grids;
	std::vector<std::string> m_Logs;
private:
	std::vector<std::vector<Particle>> m_SimulationRecord;
private:
	std::thread m_MainProcess;
	std::mutex m_MutexLock;
	float m_Progress = 0.0f;
	bool m_Started = false;
	bool m_Paused = false;
	bool m_Finished = false;
    std::string m_Filepath = "";
	SimulationLengthUnit m_LengthUnit = SimulationLengthUnit::TICKS;
	uint64_t m_SimulationLength = 0;
	bool m_EnableSafeguardCache = false;
	bool m_EnableSimulationRecord = false;
	glm::dvec2 m_Bounds = { 0, 0 };
	bool m_DynamicTimestep = false;
	uint64_t m_Timestep = 0;
	uint32_t m_NumLocalWorkers = 0;
	uint32_t m_NumRemoteWorkers = 0;
	SimulationSolver m_Solver = SimulationSolver::RKF45;
};
