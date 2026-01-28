#include "Core/Camera.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using namespace Magma;

Camera::Camera(
	glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: m_FOV(90.0f), m_AspectRatio(16.0f / 9.0f),
	m_NearPlane(0.1f), m_FarPlane(100.0f), m_OrthoSize(10.0f),
	m_Yaw(yaw), m_Pitch(pitch), m_Position(position), m_WorldUp(up),
	m_IsPerspective(true)
{

	UpdateEulerOrientation();
}

void Camera::UpdateEulerOrientation()
{
	m_Front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Front.y = sin(glm::radians(m_Pitch));
	m_Front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Front = glm::normalize(m_Front);
	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
	m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

// Updates it with m_Position, m_Front, m_Up
void Camera::UpdateViewMatrix()
{
	return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void Camera::UpdateProjectionMatrix()
{

	if (m_IsPerspective)
	{
		return glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		float orthoHeight = m_OrthoSize;
		float orthoWidth = orthoHeight * m_AspectRatio;
		return glm::ortho(-orthoWidth / 2.0f, orthoWidth / 2.0f, -orthoHeight / 2.0f, orthoHeight / 2.0f, m_NearPlane, m_FarPlane);
	}
}

void Camera::SetOrientation(const glm::vec3& front, const glm::vec3& up)
{
	m_Front = front;
	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
	m_Up = up;
}