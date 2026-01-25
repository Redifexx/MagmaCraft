#pragma once

#include <glm/glm.hpp>
#include "Core/Camera.h"

namespace Craft
{
	enum class CameraType
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

	struct CameraComponent
	{
		Magma::Camera* camera;
		CameraType cameraType;
		float FOV;
		float nearPlane;
		float farPlane;
	};
}