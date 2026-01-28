#pragma once

// bridges the magma camera object with the ECS camera component

#include <Core/Camera.h>
#include <Datatypes/EntityWorld.h>
#include <memory>

namespace Craft
{
	class CameraSystem
	{
		CameraSystem();

		public:
			void Update(EntityWorld& world);

		private:
			std::unique_ptr<Magma::Camera> m_Camera = nullptr;
	};
}