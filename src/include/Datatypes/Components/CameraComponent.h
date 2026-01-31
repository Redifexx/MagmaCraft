#pragma once

#include <glm/glm.hpp>

namespace Craft
{
	enum class CameraType
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

	struct CameraComponent
	{
		bool isPrimary = false;

		CameraType cameraType = CameraType::PERSPECTIVE;
		float FOV = 90.0f;
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
		float orthoSize = 10.0f;
		float aspectRatio = 16.0f/9.0f;

		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		glm::mat4 viewMatrix = glm::mat4(1.0f);
	};
}