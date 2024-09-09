#pragma once
#include "Simulation/Resource.h"

class Sink : public Resource {
public:
	Sink() { Initialize(); }
	Sink(uint64_t id) { Initialize(id); }
};
