#include "Quadtree.h"
#include "Core/Log.h"
#include "Renderer/Renderer.h"

#define THETA 0.5
#ifndef G
#define G 0.0000000000667430
#endif

void Quadtree::Insert(Particle* particle) {
    if (!m_Boundary.Contains(particle)) return;
    if (m_Leaf) {
        if (!m_Particle) {
            m_Particle = particle;
            m_CenterOfMass = *particle;
            return;
        } else {
            Subdivide();
            InsertIntoChildren(particle);
            m_Particle = nullptr;
            m_Leaf = false;
            return;
        }
    }
    InsertIntoChildren(particle);
}

void Quadtree::CalculateCenterOfMass() {
    if (m_Leaf && m_Particle) {
        m_CenterOfMass = *m_Particle;
        return;
    }

    if (m_NW) m_NW->CalculateCenterOfMass();
    if (m_NE) m_NE->CalculateCenterOfMass();
    if (m_SW) m_SW->CalculateCenterOfMass();
    if (m_SE) m_SE->CalculateCenterOfMass();

    double totalMass = 0;
    double cmX = 0;
    double cmY = 0;
    for (auto& child : { m_NW.get(), m_NE.get(), m_SW.get(), m_SE.get() }) {
        if (child && child->m_CenterOfMass.Mass() > 0) {
            totalMass += child->m_CenterOfMass.Mass();
            cmX += child->m_CenterOfMass.Mass() * child->m_CenterOfMass.Position().x;
            cmY += child->m_CenterOfMass.Mass() * child->m_CenterOfMass.Position().y;
        }
    }
    if (totalMass > 0) {
        m_CenterOfMass.SetMass(totalMass);
        m_CenterOfMass.SetPosition({ cmX / totalMass, cmY / totalMass, 0.0 });
    }
}

void Quadtree::SerialCalculateForce(Particle &p, double unitsize) {
    if (m_Leaf) {
        if (m_Particle && m_Particle != &p)
            SerialApplyForce(p, *m_Particle, unitsize);
        return;
    }
    double dx = unitsize * (m_CenterOfMass.Position().x - p.Position().x);
    double dy = unitsize * (m_CenterOfMass.Position().y - p.Position().y);
    double distance = std::sqrt(dx*dx + dy*dy);
    if (m_Boundary.radius / distance < THETA) {
        Particle vm = m_CenterOfMass;
        SerialApplyForce(p, vm, unitsize);
    } else {
        if (m_NW) m_NW->SerialCalculateForce(p, unitsize);
        if (m_NE) m_NE->SerialCalculateForce(p, unitsize);
        if (m_SW) m_SW->SerialCalculateForce(p, unitsize);
        if (m_SE) m_SE->SerialCalculateForce(p, unitsize);
    }
}

void Quadtree::SerialApplyForce(Particle &p, Particle &other, double unitsize) {
    double dx = unitsize * (other.Position().x - p.Position().x);
    double dy = unitsize * (other.Position().y - p.Position().y);
	double inv_r3 = std::pow((dx*dx) + (dy*dy) + (3*3), -1.5);
	glm::dvec3 pa = { 0, 0, 0 };
    glm::dvec3 olda = p.Acceleration();
	pa.x = olda.x + (G * (dx * inv_r3) * other.Mass());
	pa.y = olda.y + (G * (dy * inv_r3) * other.Mass());
    p.SetAcceleration(pa);
}

void Quadtree::DrawTree() {
	LineProperties lp = Renderer::GetLineProperties();
	Color lc = lp.LineColor;
	lp.LineColor = { 255, 0, 0, 255 };
	Renderer::SetLineProperties(lp);
	DrawBoundary();
    lp.LineColor = lc;
	Renderer::SetLineProperties(lp);
}

void Quadtree::DrawBoundary() {
    if (m_Boundary.radius <= 0) return;
	Line line1(
        {m_Boundary.x - m_Boundary.radius, m_Boundary.y + m_Boundary.radius, 0}, 
        {m_Boundary.x + m_Boundary.radius, m_Boundary.y + m_Boundary.radius, 0});
	Line line2( 
        {m_Boundary.x + m_Boundary.radius, m_Boundary.y + m_Boundary.radius, 0},
        {m_Boundary.x + m_Boundary.radius, m_Boundary.y - m_Boundary.radius, 0});
	Line line3(
        {m_Boundary.x + m_Boundary.radius, m_Boundary.y - m_Boundary.radius, 0},
        {m_Boundary.x - m_Boundary.radius, m_Boundary.y - m_Boundary.radius, 0});
	Line line4(
        {m_Boundary.x - m_Boundary.radius, m_Boundary.y - m_Boundary.radius, 0}, 
        {m_Boundary.x - m_Boundary.radius, m_Boundary.y + m_Boundary.radius, 0});
    Renderer::DrawLine(line1);
    Renderer::DrawLine(line2);
    Renderer::DrawLine(line3);
    Renderer::DrawLine(line4);
    if (!m_Leaf) {
        if (m_NW) m_NW->DrawBoundary();
        if (m_NE) m_NE->DrawBoundary();
        if (m_SW) m_SW->DrawBoundary();
        if (m_SE) m_SE->DrawBoundary();
    }
}

void Quadtree::Subdivide() {
    m_NW = CreateScope<Quadtree>(m_Boundary.NW());
    m_NE = CreateScope<Quadtree>(m_Boundary.NE());
    m_SW = CreateScope<Quadtree>(m_Boundary.SW());
    m_SE = CreateScope<Quadtree>(m_Boundary.SE());
}

void Quadtree::InsertIntoChildren(Particle* particle) {
    if (m_NW->m_Boundary.Contains(particle)) m_NW->Insert(particle);
    if (m_NE->m_Boundary.Contains(particle)) m_NE->Insert(particle);
    if (m_SW->m_Boundary.Contains(particle)) m_SW->Insert(particle);
    if (m_SE->m_Boundary.Contains(particle)) m_SE->Insert(particle);
}
