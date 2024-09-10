#pragma once
#include "Simulation/Grid.h"
#include "Simulation/Particle.h"
#include "Simulation/Sink.h"
#include "Simulation/Source.h"
#include "Core/Safety.h"
#include <vector>

enum class SimulationLengthUnit {
	TICKS = 0,
	MICROSECONDS = 1,
	MILLISECONDS = 2,
	SECONDS = 3,
};

enum class SimulationSolver {
	RKF45 = 0,
	EULER = 1,
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
private:
    std::vector<Ref<Source>> m_Sources;
    std::vector<Ref<Sink>> m_Sinks;
    std::vector<Ref<Particle>> m_Particles;
    std::vector<Ref<Grid>> m_Grids;
private:
    std::string m_Filepath = "";
	SimulationLengthUnit m_LengthUnit = SimulationLengthUnit::TICKS;
	uint64_t m_SimulationLength = 0;
	bool m_EnableSafeguardCache = false;
	bool m_EnableSimulationRecord = false;
	SimulationSolver m_Solver = SimulationSolver::RKF45;
};
