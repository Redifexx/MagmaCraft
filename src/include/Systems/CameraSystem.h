#pragma once

// bridges the magma camera object with the ECS camera component

#include <Core/Camera.h>
#include <Datatypes/EntityWorld.h>

namespace Craft
{
	class CameraSystem
	{
		// get all entites with transform and relationship components
		SparseSet<TransformComponent>* transformPool = world.GetComponentPool<TransformComponent>();
		SparseSet<CameraComponent>* cameraPool = world.GetComponentPool<CameraComponent>();

		if (!transformPool || !cameraPool) return;

		public:
			void Update(EntityWorld& world);
	};
}