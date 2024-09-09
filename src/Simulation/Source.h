#pragma once
#include "Simulation/Resource.h"

class Source : public Resource {
public:
	Source() { Initialize(); }
	Source(uint64_t id) { Initialize(id); }
};
