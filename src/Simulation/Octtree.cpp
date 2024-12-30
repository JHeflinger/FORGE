#include "Octtree.h"
#include "Core/Log.h"
#include "Renderer/Renderer.h"

#define THETA 0.5
#ifndef G
#define G 0.0000000000667430
#endif

void Octtree::GetLeaves(std::vector<Octtree*>* leaves) {
    if (m_Leaf) {
        leaves->push_back(this);
    } else {
        m_TNW->GetLeaves(leaves);
        m_TNE->GetLeaves(leaves);
        m_TSW->GetLeaves(leaves);
        m_TSE->GetLeaves(leaves);
        m_BNW->GetLeaves(leaves);
        m_BNE->GetLeaves(leaves);
        m_BSW->GetLeaves(leaves);
        m_BSE->GetLeaves(leaves);
    }
}

void Octtree::Insert(Particle* particle) {
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

void Octtree::CalculateCenterOfMass() {
    if (m_Leaf && m_Particle) {
        m_CenterOfMass = *m_Particle;
        return;
    }

    if (m_TNW) m_TNW->CalculateCenterOfMass();
    if (m_TNE) m_TNE->CalculateCenterOfMass();
    if (m_TSW) m_TSW->CalculateCenterOfMass();
    if (m_TSE) m_TSE->CalculateCenterOfMass();
    if (m_BNW) m_BNW->CalculateCenterOfMass();
    if (m_BNE) m_BNE->CalculateCenterOfMass();
    if (m_BSW) m_BSW->CalculateCenterOfMass();
    if (m_BSE) m_BSE->CalculateCenterOfMass();

    double totalMass = 0;
    double cmX = 0;
    double cmY = 0;
	double cmZ = 0;
    for (auto& child : { m_BNW.get(), m_BNE.get(), m_BSW.get(), m_BSE.get(), m_TNW.get(), m_TNE.get(), m_TSW.get(), m_TSE.get() }) {
        if (child && child->m_CenterOfMass.Mass() > 0) {
            totalMass += child->m_CenterOfMass.Mass();
            cmX += child->m_CenterOfMass.Mass() * child->m_CenterOfMass.Position().x;
            cmY += child->m_CenterOfMass.Mass() * child->m_CenterOfMass.Position().y;
			cmZ += child->m_CenterOfMass.Mass() * child->m_CenterOfMass.Position().z;
        }
    }
    if (totalMass > 0) {
        m_CenterOfMass.SetMass(totalMass);
        m_CenterOfMass.SetPosition({ cmX / totalMass, cmY / totalMass, cmZ / totalMass });
    }
}

void Octtree::SerialCalculateForce(Particle &p, double unitsize) {
    if (m_Leaf) {
        if (m_Particle && m_Particle != &p)
            SerialApplyForce(p, *m_Particle, unitsize);
        return;
    }
    double dx = unitsize * (m_CenterOfMass.Position().x - p.Position().x);
    double dy = unitsize * (m_CenterOfMass.Position().y - p.Position().y);
    double dz = unitsize * (m_CenterOfMass.Position().z - p.Position().z);
    double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (m_Boundary.radius / distance < THETA) {
        Particle vm = m_CenterOfMass;
        SerialApplyForce(p, vm, unitsize);
    } else {
        if (m_TNW) m_TNW->SerialCalculateForce(p, unitsize);
        if (m_TNE) m_TNE->SerialCalculateForce(p, unitsize);
        if (m_TSW) m_TSW->SerialCalculateForce(p, unitsize);
        if (m_TSE) m_TSE->SerialCalculateForce(p, unitsize);
        if (m_BNW) m_BNW->SerialCalculateForce(p, unitsize);
        if (m_BNE) m_BNE->SerialCalculateForce(p, unitsize);
        if (m_BSW) m_BSW->SerialCalculateForce(p, unitsize);
        if (m_BSE) m_BSE->SerialCalculateForce(p, unitsize);
    }
}

void Octtree::SerialApplyForce(Particle &p, Particle &other, double unitsize) {
    double dx = unitsize * (other.Position().x - p.Position().x);
    double dy = unitsize * (other.Position().y - p.Position().y);
    double dz = unitsize * (other.Position().z - p.Position().z);
	double inv_r3 = std::pow((dx*dx) + (dy*dy) + (dz*dz) + (3*3), -1.5);
	glm::dvec3 pa = { 0, 0, 0 };
    glm::dvec3 olda = p.Acceleration();
	pa.x = olda.x + (G * (dx * inv_r3) * other.Mass());
	pa.y = olda.y + (G * (dy * inv_r3) * other.Mass());
	pa.z = olda.z + (G * (dz * inv_r3) * other.Mass());
    p.SetAcceleration(pa);
}

void Octtree::Subdivide() {
    m_TNW = CreateScope<Octtree>(m_Boundary.TNW(), m_SizeRef);
    m_TNE = CreateScope<Octtree>(m_Boundary.TNE(), m_SizeRef);
    m_TSW = CreateScope<Octtree>(m_Boundary.TSW(), m_SizeRef);
    m_TSE = CreateScope<Octtree>(m_Boundary.TSE(), m_SizeRef);
    m_BNW = CreateScope<Octtree>(m_Boundary.BNW(), m_SizeRef);
    m_BNE = CreateScope<Octtree>(m_Boundary.BNE(), m_SizeRef);
    m_BSW = CreateScope<Octtree>(m_Boundary.BSW(), m_SizeRef);
    m_BSE = CreateScope<Octtree>(m_Boundary.BSE(), m_SizeRef);
    if (m_SizeRef != nullptr) (*m_SizeRef)--;
}

void Octtree::InsertIntoChildren(Particle* particle) {
    if (m_TNW->m_Boundary.Contains(particle)) m_TNW->Insert(particle);
    if (m_TNE->m_Boundary.Contains(particle)) m_TNE->Insert(particle);
    if (m_TSW->m_Boundary.Contains(particle)) m_TSW->Insert(particle);
    if (m_TSE->m_Boundary.Contains(particle)) m_TSE->Insert(particle);
    if (m_BNW->m_Boundary.Contains(particle)) m_BNW->Insert(particle);
    if (m_BNE->m_Boundary.Contains(particle)) m_BNE->Insert(particle);
    if (m_BSW->m_Boundary.Contains(particle)) m_BSW->Insert(particle);
    if (m_BSE->m_Boundary.Contains(particle)) m_BSE->Insert(particle);
}
