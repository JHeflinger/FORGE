#pragma once
#include "Resource.h"

class Source : public Resource {
public:
	Source() { Initialize(); }
	Source(uint64_t id) { Initialize(id); }
};
