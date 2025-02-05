#pragma once
#include "Simulation/Simulation.h"
#include <iostream>
#include <memory>
#include <string>

class Network {
public:
	Network(Simulation* context) : m_SimulationRef(context) {}
	~Network() {}
private:
	Simulation* m_SimulationRef = nullptr;
};
