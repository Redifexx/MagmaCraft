#pragma once

// bridges the magma camera object with the ECS camera component

#include <Core/Camera.h>
#include <Datatypes/EntityWorld.h>
#include <memory>
#include <glm/glm.hpp>

// Updates all cameras in the world
namespace Craft
{
	class CameraSystem
	{
		public:
			void Update(EntityWorld& world);
	};
}