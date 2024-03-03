#pragma once
#include "../Core/Timestep.h"
#include "../Events/Event.h"
#include "glm/glm.hpp"

enum class CameraTypes {
	PERSPECTIVE = 0,
	ORTHOGRAPHIC = 1,
};

struct CameraProperties {
	float FOV;
	float NearClip;
	float FarClip;
	glm::vec2 ViewportDimensions;
	glm::mat4 Projection;
	glm::mat4 View;
	CameraTypes Type;
};

class Camera {
public:
	Camera() { Reset(); }
	const glm::mat4& GetViewProjection() const { return GetProjection(); }//{ return m_Properties.Projection * m_Properties.View; }
	const glm::mat4& GetProjection() const { return m_Properties.Projection; }
public:
	void OnUpdate(Timestep ts, bool updateControl = true);
	void OnEvent(Event& e);
public:
	void UpdateView();
	void UpdateProjection();
public:
	void Reset();
private:
	CameraProperties m_Properties;
};
