#pragma once
#include "Simulation/Resource.h"
#include "glm/glm.hpp"

class Particle : public Resource {
public:
	Particle() { Initialize(); }
	Particle(uint64_t id) { Initialize(id); }
public:
	glm::vec3 Position() { return m_Position; }
	void SetPosition(glm::vec3 position) { m_Position = position; }
	glm::vec3& RawPosition() { return m_Position; }
	glm::vec3 Velocity() { return m_Velocity; }
	void SetVelocity(glm::vec3 velocity) { m_Velocity = velocity; }
	glm::vec3& RawVelocity() { return m_Velocity; }
	float Mass() { return m_Mass; }
	void SetMass(float mass) { m_Mass = mass; }
	float* RawMass() { return &m_Mass; }
	float Radius() { return m_Radius; }
	void SetRadius(float radius) { m_Radius = radius; }
	float* RawRadius() { return &m_Radius; }
public:
	glm::vec3 Acceleration() { return m_Acceleration; }
	void SetAcceleration(glm::vec3 acceleration) { m_Acceleration = acceleration; }
private:
	glm::vec3 m_Acceleration = { 0, 0, 0 };
	glm::vec3 m_Position = { 0, 0, 0 };
	glm::vec3 m_Velocity = { 0, 0, 0 };
	float m_Mass;
	float m_Radius = 0.5f;
};
