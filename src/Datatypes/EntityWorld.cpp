#include "Datatypes/EntityWorld.h"

using namespace Craft;

EntityWorld::EntityWorld()
{
	// Create component pools for common components
	GetComponentPool<TransformComponent>();			// 0
	GetComponentPool<RelationshipComponent>();		// 1
	GetComponentPool<ModelComponent>();				// 2
	GetComponentPool<CameraComponent>();			// 3
	GetComponentPool<HealthComponent>();			// 4

}

EntityWorld::~EntityWorld()
{
	// clean up all component pools
	for (ISparseSet* pool : m_ComponentPools)
	{
		if (pool)
		{
			delete pool;
		}
	}
}
