#include "Camera.h"
#include "Events/Input.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

void Camera::OnUpdate(Timestep ts) {
	if (Input::IsKeyPressed(KeyCode::Space) && m_Properties.Focused) {
		const glm::vec2& mouse = Input::GetMousePosition();
		glm::vec2 delta = (mouse - m_Properties.MousePosition) * 0.003f;
		m_Properties.MousePosition = mouse;
		if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft))
			MousePan(delta);
		else if (Input::IsMouseButtonPressed(MouseCode::ButtonRight))
			MouseRotate(delta);
		else if (Input::IsMouseButtonPressed(MouseCode::ButtonMiddle))
			MouseZoom(delta.y); //TODO: use x axis as well so its not just based on vertical axis?
		if (Input::IsKeyPressed(KeyCode::GraveAccent))
			SoftReset();
		if (Input::IsKeyPressed(KeyCode::LeftAlt)) {
			if (m_Properties.ToggleBuffer) {
				if (m_Properties.Type == CameraTypes::PERSPECTIVE)
					m_Properties.Type = CameraTypes::ORTHOGRAPHIC;
				else 
					m_Properties.Type = CameraTypes::PERSPECTIVE;
				m_Properties.ToggleBuffer = false;
				UpdateProjection();
			}
		} else m_Properties.ToggleBuffer = true;
	}
	UpdateView();
}

void Camera::OnEvent(Event& e) {
	if (m_Properties.Focused) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(Camera::OnMouseScroll));
	}
}

bool Camera::OnMouseScroll(MouseScrolledEvent& e) {
	float delta = e.GetYOffset() * 0.1f;
	MouseZoom(delta);
	UpdateView();
	return false;
}

void Camera::UpdateView() {
	m_Properties.Position = CalculatePosition();
	glm::quat orientation = GetOrientation();
	m_Properties.View = glm::translate(glm::mat4(1.0f), m_Properties.Position) * glm::toMat4(orientation);
	m_Properties.View = glm::inverse(m_Properties.View);
}

void Camera::UpdateProjection() {
	float aspectratio = m_Properties.ViewportDimensions.x / m_Properties.ViewportDimensions.y;
	if (m_Properties.Type == CameraTypes::PERSPECTIVE) {
		m_Properties.Projection = glm::perspective(glm::radians(m_Properties.FOV), 
												   aspectratio,
												   m_Properties.NearClip,
												   m_Properties.FarClip);
	} else {
		m_Properties.Projection = glm::ortho(-m_Properties.OrthographicSize * aspectratio * 0.5f,
											 m_Properties.OrthographicSize * aspectratio * 0.5f,
											 -m_Properties.OrthographicSize * 0.5f,
											 m_Properties.OrthographicSize * 0.5f,
											 m_Properties.OrthographicNear,
											 m_Properties.OrthographicFar);
	}
}

glm::vec3 Camera::CalculatePosition() const {
	return m_Properties.FocalPoint - GetForwardDirection() * m_Properties.Distance;
}

glm::quat Camera::GetOrientation() const {
	return glm::quat(glm::vec3(-m_Properties.Pitch, -m_Properties.Yaw, 0.0f));
}

glm::vec3 Camera::GetForwardDirection() const {
	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 Camera::GetUpDirection() const {
	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Camera::GetRightDirection() const {
	return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

float Camera::ZoomSpeed() const {
	float distance = m_Properties.Distance * 0.5f;
	distance = std::max(distance, 0.0f);
	float speed = distance * distance;
	speed = std::min(speed, 100.0f); // max speed = 100
	return speed;
}

glm::vec2 Camera::PanSpeed() const {
	float x = std::min(m_Properties.ViewportDimensions.x / 10000.0f, 2.4f); // max = 2.4f
	float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
	float y = std::min(m_Properties.ViewportDimensions.y / 10000.0f, 2.4f); // max = 2.4f
	float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;
	return { xFactor, yFactor };
}

float Camera::RotationSpeed() const {
	return 0.8f;
}

void Camera::MouseZoom(float delta) {
	if (m_Properties.Type == CameraTypes::PERSPECTIVE) {
		m_Properties.Distance -= delta * ZoomSpeed();
		if (m_Properties.Distance < 1.0f) {
			m_Properties.FocalPoint += GetForwardDirection();
			m_Properties.Distance = 1.0f;
		}
	} else {
		m_Properties.OrthographicSize -= delta* ZoomSpeed();
		UpdateProjection();
	}
}

void Camera::MousePan(const glm::vec2& delta) {
	glm::vec2 panspeed = PanSpeed();
	if (m_Properties.Type == CameraTypes::ORTHOGRAPHIC)
		panspeed = { 1.0f, 1.0f };
	m_Properties.FocalPoint += -GetRightDirection() * (delta.x * panspeed.x * m_Properties.Distance);
	m_Properties.FocalPoint += GetUpDirection() * (delta.y * panspeed.y * m_Properties.Distance);
}

void Camera::MouseRotate(const glm::vec2& delta) {
	float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
	m_Properties.Yaw += yawSign * delta.x * RotationSpeed();
	m_Properties.Pitch += delta.y * RotationSpeed();
}

void Camera::Reset() {
	m_Properties.FOV = 45.0f;
	m_Properties.NearClip = 0.1f;
	m_Properties.FarClip = 1000.0f;
	m_Properties.ViewportDimensions = { 1280.0f, 720.0f };
	m_Properties.Type = CameraTypes::PERSPECTIVE;
	m_Properties.OrthographicSize = 10.0f;
	m_Properties.OrthographicNear = -9999.0f;
	m_Properties.OrthographicFar = 9999.0f;
	m_Properties.Position = { 0.0f, 0.0f, 0.0f };
	m_Properties.FocalPoint = { 0.0f, 0.0f, 0.0f };
	m_Properties.MousePosition = { 0.0f, 0.0f };
	m_Properties.Pitch = 0.0f;
	m_Properties.Yaw = 0.0f;
	m_Properties.Distance = 10.0f;
	m_Properties.ToggleBuffer = true;
	m_Properties.Focused = false;
	
	UpdateProjection();
	UpdateView();
}

void Camera::SoftReset() {
	m_Properties.Pitch = 0.0f;
	m_Properties.Yaw = 0.0f;
	m_Properties.Distance = 10.0f;
	m_Properties.Position = { 0.0f, 0.0f, 0.0f };
	m_Properties.FocalPoint = { 0.0f, 0.0f, 0.0f };
}
