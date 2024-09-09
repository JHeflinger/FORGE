#pragma once
#include "Utils/ObjectUtils.h"
#include <string>

class Resource {
public:
	virtual ~Resource() = default;
protected:
	virtual void Initialize() { m_ID = ObjectUtils::NewID(); }
	virtual void Initialize(uint64_t id) { m_ID = ObjectUtils::SetID(id); }
public:
	inline uint64_t ID() { return m_ID; }
	inline std::string Name() { return m_Name; }
public:
	void SetName(std::string name) { m_Name = name; }
private:
	std::string m_Name = "Untitled";
	uint64_t m_ID = 0;
};
