#pragma once
#include "Resource.h"

class Grid : public Resource {
public:
	Grid() { Initialize(); }
	Grid(uint64_t id) { Initialize(id); }
};
