#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

void Camera::Reset() {
	m_Properties.FOV = 45.0f;
	m_Properties.NearClip = 0.1f;
	m_Properties.FarClip = 1000.0f;
	m_Properties.ViewportDimensions = { 1280, 720 };
	m_Properties.Type = CameraTypes::PERSPECTIVE;

	m_Properties.Projection = glm::mat4(1.0f);
	m_Properties.View = glm::mat4(1.0f);
	//UpdateProjection();
	//UpdateView();
}

void Camera::UpdateProjection() {
	float aspectratio = m_Properties.ViewportDimensions.x / m_Properties.ViewportDimensions.y;
	if (m_Properties.Type == CameraTypes::PERSPECTIVE) {
		m_Properties.Projection = glm::perspective(glm::radians(m_Properties.FOV), 
												   aspectratio,
												   m_Properties.NearClip,
												   m_Properties.FarClip);
	} else {
		
	}
}

void Camera::UpdateView() {

}
