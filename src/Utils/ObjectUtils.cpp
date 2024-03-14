#include "ObjectUtils.h"

static uint64_t s_ThresholdID = 0;

uint64_t ObjectUtils::NewID() {
	s_ThresholdID++;
	return s_ThresholdID;
}

uint64_t ObjectUtils::SetID(uint64_t id) {
	s_ThresholdID = id;
	return id;
}