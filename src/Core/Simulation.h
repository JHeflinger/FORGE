#pragma once
#include <vector>

struct Source {

};

struct Sink {

};

struct Particle {

};

struct Grid {

};

class Simulation {
public:
private:
    std::vector<Source> m_Sources;
    std::vector<Source> m_Sinks;
    std::vector<Particle> m_Particles;
    std::vector<Grid> m_Grids;
};