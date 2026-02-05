#pragma once
#include <string>

// won't have much use at until AABB collisions are added
namespace Craft
{
	struct PhysicsComponent
	{
		glm::vec3 velocity;
		bool hasGravity = false;
	};
}