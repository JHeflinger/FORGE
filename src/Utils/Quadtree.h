#pragma once
#include "Simulation/Particle.h"
#include "Core/Safety.h"

struct Quad {
    double x;
    double y;
    double radius;
    bool Contains(Particle* particle) {
        return particle->Position().x >= x - radius &&
            particle->Position().x < x + radius &&
            particle->Position().y >= y - radius &&
            particle->Position().y < y + radius;
    }
    Quad NW() const { return {x - radius / 2, y + radius / 2, radius / 2}; }
    Quad NE() const { return {x + radius / 2, y + radius / 2, radius / 2}; }
    Quad SW() const { return {x - radius / 2, y - radius / 2, radius / 2}; }
    Quad SE() const { return {x + radius / 2, y - radius / 2, radius / 2}; }
};

class Quadtree {
public:
    Quadtree(const Quad &boundary) : m_Boundary(boundary) {}
public:
    void Insert(Particle* particle);
    void CalculateCenterOfMass();
public:
    void SerialCalculateForce(Particle &p, double unitsize);
    void SerialApplyForce(Particle &p, Particle &other, double unitsize);
    void DrawTree();
    void DrawBoundary();
private:
    void Subdivide();
    void InsertIntoChildren(Particle* particle);
public:
    Quad m_Boundary = { 0 };
    Particle m_CenterOfMass;
    bool m_Leaf = true;
    Scope<Quadtree> m_NW = nullptr;
    Scope<Quadtree> m_NE = nullptr;
    Scope<Quadtree> m_SW = nullptr;
    Scope<Quadtree> m_SE = nullptr;
    Particle* m_Particle = nullptr;
};