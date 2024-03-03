#pragma once
#include "../Core/Timestep.h"
#include "../Events/Event.h"
#include "../Events/MouseEvent.h"
#include "glm/glm.hpp"

enum class CameraTypes {
	PERSPECTIVE = 0,
	ORTHOGRAPHIC = 1,
};

struct CameraProperties {
	float FOV;
	float NearClip;
	float FarClip;
	float OrthographicSize;
	float OrthographicNear;
	float OrthographicFar;
	float Pitch;
	float Yaw;
	float Distance;
	glm::vec2 ViewportDimensions;
	glm::vec3 Position;
	glm::vec3 FocalPoint;
	glm::mat4 Projection;
	glm::mat4 View;
	CameraTypes Type;
};

class Camera {
public:
	Camera() { Reset(); }
	const glm::mat4 GetViewProjection() const { return m_Properties.Projection * m_Properties.View; }
	const glm::mat4& GetProjection() const { return m_Properties.Projection; }
	const glm::vec3& GetPosition() const { return m_Properties.Position; }
public:
	void OnUpdate(Timestep ts, bool updateControl = true);
	void OnEvent(Event& e);
	bool OnMouseScroll(MouseScrolledEvent& e);
public:
	void UpdateView();
	void UpdateProjection();
private:
	glm::quat GetOrientation() const;
	glm::vec3 CalculatePosition() const;
	glm::vec3 GetForwardDirection() const;
	float ZoomSpeed() const;
private:
	void MouseZoom(float delta);
public:
	void Reset();
private:
	CameraProperties m_Properties;
};
