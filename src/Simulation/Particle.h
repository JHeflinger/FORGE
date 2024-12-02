#pragma once
#include "Simulation/Resource.h"
#include "glm/glm.hpp"

class Particle : public Resource {
public:
	Particle() { Initialize(); }
	Particle(uint64_t id) { Initialize(id); }
public:
	glm::dvec3 Position() { return m_Position; }
	void SetPosition(glm::dvec3 position) { m_Position = position; }
	glm::dvec3& RawPosition() { return m_Position; }
	glm::dvec3 Velocity() { return m_Velocity; }
	void SetVelocity(glm::dvec3 velocity) { m_Velocity = velocity; }
	glm::dvec3& RawVelocity() { return m_Velocity; }
	double Mass() { return m_Mass; }
	void SetMass(double mass) { m_Mass = mass; }
	double* RawMass() { return &m_Mass; }
	double Radius() { return m_Radius; }
	void SetRadius(double radius) { m_Radius = radius; }
	double* RawRadius() { return &m_Radius; }
public:
	glm::dvec3 Acceleration() { return m_Acceleration; }
	void SetAcceleration(glm::dvec3 acceleration) { m_Acceleration = acceleration; }
private:
	glm::dvec3 m_Acceleration = { 0, 0, 0 };
	glm::dvec3 m_Position = { 0, 0, 0 };
	glm::dvec3 m_Velocity = { 0, 0, 0 };
	double m_Mass = 0.0;
	double m_Radius = 0.5f;
};
