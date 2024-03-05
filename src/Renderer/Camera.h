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
	glm::vec2 MousePosition;
	glm::vec2 ViewportDimensions;
	glm::vec3 Position;
	glm::vec3 FocalPoint;
	glm::mat4 Projection;
	glm::mat4 View;
	CameraTypes Type;
	bool ToggleBuffer;
};

class Camera {
public:
	Camera() { Reset(); }
	const glm::mat4 GetViewProjection() const { return m_Properties.Projection * m_Properties.View; }
	const glm::mat4& GetProjection() const { return m_Properties.Projection; }
	const glm::vec3& GetPosition() const { return m_Properties.Position; }
public:
	inline void SetViewportSize(float width, float height) { m_Properties.ViewportDimensions.x = width; m_Properties.ViewportDimensions.y = height; UpdateProjection(); }
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
	glm::vec3 GetUpDirection() const;
	glm::vec3 GetRightDirection() const;
	float ZoomSpeed() const;
	glm::vec2 PanSpeed() const;
	float RotationSpeed() const;
private:
	void MouseZoom(float delta);
	void MousePan(const glm::vec2& delta);
	void MouseRotate(const glm::vec2& delta);
public:
	void Reset();
	void SoftReset();
private:
	CameraProperties m_Properties;
};
