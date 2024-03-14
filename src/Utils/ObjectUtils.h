#pragma once
#include <cstdint>

class ObjectUtils {
public:
	static uint64_t NewID();
	static uint64_t SetID(uint64_t id);
};
