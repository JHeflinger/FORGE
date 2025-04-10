#pragma once
#include "Simulation/Particle.h"
#include "Core/Safety.h"

#define GRAVITY 0.0000000000667430
#define THETA 0.5

struct Oct {
    double x;
    double y;
	double z;
    double radius;
    bool Contains(Particle* particle) {
        return particle->Position().x >= x - radius &&
            particle->Position().x < x + radius &&
            particle->Position().y >= y - radius &&
            particle->Position().y < y + radius &&
			particle->Position().z >= z - radius &&
			particle->Position().z < z + radius;
    }
    Oct TNW() const { return {x - radius / 2, y + radius / 2, z + radius / 2, radius / 2}; }
    Oct TNE() const { return {x + radius / 2, y + radius / 2, z + radius / 2, radius / 2}; }
    Oct TSW() const { return {x - radius / 2, y - radius / 2, z + radius / 2, radius / 2}; }
    Oct TSE() const { return {x + radius / 2, y - radius / 2, z + radius / 2, radius / 2}; }
    Oct BNW() const { return {x - radius / 2, y + radius / 2, z - radius / 2, radius / 2}; }
    Oct BNE() const { return {x + radius / 2, y + radius / 2, z - radius / 2, radius / 2}; }
    Oct BSW() const { return {x - radius / 2, y - radius / 2, z - radius / 2, radius / 2}; }
    Oct BSE() const { return {x + radius / 2, y - radius / 2, z - radius / 2, radius / 2}; }
};

class Octtree {
public:
    Octtree() {};
    Octtree(const Oct &boundary, size_t* sizeref) { m_Boundary = boundary; m_SizeRef = sizeref; if (m_SizeRef != nullptr) (*m_SizeRef)++; }
    bool Contains(Particle* particle) { return m_Boundary.Contains(particle); }
public:
    void GetLeaves(std::vector<Octtree*>* leaves);
public:
    void Insert(Particle* particle);
    void CalculateCenterOfMass();
public:
    void SerialCalculateForce(Particle &p, double unitsize);
    void SerialApplyForce(Particle &p, Particle &other, double unitsize);
public:
    void AsList(std::vector<std::pair<Oct, Particle*>>* list);
private:
    void Subdivide();
    void InsertIntoChildren(Particle* particle);
public:
    size_t* m_SizeRef = nullptr;
    Oct m_Boundary;
    Particle m_CenterOfMass;
    bool m_Leaf = true;
    Scope<Octtree> m_TNW = nullptr;
    Scope<Octtree> m_TNE = nullptr;
    Scope<Octtree> m_TSW = nullptr;
    Scope<Octtree> m_TSE = nullptr;
    Scope<Octtree> m_BNW = nullptr;
    Scope<Octtree> m_BNE = nullptr;
    Scope<Octtree> m_BSW = nullptr;
    Scope<Octtree> m_BSE = nullptr;
    Particle* m_Particle = nullptr;
};
