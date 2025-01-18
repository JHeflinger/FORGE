#pragma once

struct SimulationDetails {
	uint64_t length;
    uint32_t unit;
	bool safeguard;
	bool record;
	uint32_t solver;
	double boundx;
	double boundy;
	uint64_t timestep;
};