#pragma once

#include <Datatypes/EntityWorld.h>

namespace Craft
{
	struct TransformSystem
	{
		public:
			void Update(EntityWorld& world);
		private:
			void UpdateWorldMatrix(EntityWorld& world, uint32_t entityID, const glm::mat4& parentMatrix, bool isParentDirty);
	};
}