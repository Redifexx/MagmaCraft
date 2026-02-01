#pragma once
#include <string>

namespace Craft
{
	struct PhysicsComponent
	{
		glm::vec3 velocity;
		bool hasGravity = false;
	};
}