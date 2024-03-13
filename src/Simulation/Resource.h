#pragma once
#include "../Utils/ObjectUtils.h"

class Resource {
public:
	virtual ~Resource() = default;
protected:
	virtual void Initialize() { m_ID = ObjectUtils::NewID(); }
public:
	inline uint64_t ID() { return m_ID; }
private:
	uint64_t m_ID = 0;
};
