#pragma once
#include "glm/glm.hpp"

typedef glm::mat4 Projection;

class Camera {
public:
	Camera() = default;
	Camera(const Projection& projection) : m_Projection(projection) {}
	const Projection& GetProjection() const { return m_Projection; }
private:
	Projection m_Projection = Projection(1.0f);
}
