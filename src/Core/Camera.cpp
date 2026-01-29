#include "Core/Camera.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using namespace Magma;


void Camera::UpdateProjectionMatrix()
{

	if (m_IsPerspective)
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		float orthoHeight = m_OrthoSize;
		float orthoWidth = orthoHeight * m_AspectRatio;
		m_ProjectionMatrix = glm::ortho(-orthoWidth / 2.0f, orthoWidth / 2.0f, -orthoHeight / 2.0f, orthoHeight / 2.0f, m_NearPlane, m_FarPlane);
	}
}