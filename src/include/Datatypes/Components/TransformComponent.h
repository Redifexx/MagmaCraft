#pragma once

#include <glm/glm.hpp>

namespace Craft
{
	struct TransformComponent
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
	};
}