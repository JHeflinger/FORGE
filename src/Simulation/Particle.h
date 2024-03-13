#pragma once
#include "Resource.h"
#include "glm/glm.hpp"

class Particle : public Resource {
public:
	Particle() { Initialize(); }
public:
	glm::vec3 Position() { return m_Position; }
	void SetPosition(glm::vec3 position) { m_Position = position; }
	glm::vec3& RawPosition() { return m_Position; }
private:
	glm::vec3 m_Position;
};
