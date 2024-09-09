#pragma once
#include "Simulation/Grid.h"
#include "Simulation/Particle.h"
#include "Simulation/Sink.h"
#include "Simulation/Source.h"
#include "Core/Safety.h"
#include <vector>

class Simulation {
public:
    std::vector<Ref<Source>>& Sources() { return m_Sources; }
    std::vector<Ref<Sink>>& Sinks() { return m_Sinks; }
    std::vector<Ref<Particle>>& Particles() { return m_Particles; }
    std::vector<Ref<Grid>>& Grids() { return m_Grids; }
public:
    std::string Filepath() { return m_Filepath; }
    void SetFilepath(std::string path) { m_Filepath = path; }
private:
    std::vector<Ref<Source>> m_Sources;
    std::vector<Ref<Sink>> m_Sinks;
    std::vector<Ref<Particle>> m_Particles;
    std::vector<Ref<Grid>> m_Grids;
private:
    std::string m_Filepath = "";
};
