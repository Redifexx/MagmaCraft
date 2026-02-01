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
	GetComponentPool<PlayerComponent>();			// 5
	GetComponentPool<PhysicsComponent>();			// 6
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

uint32_t EntityWorld::AddEntity()
{
	Entity newEntity;
	newEntity.id = static_cast<uint32_t>(m_Entities.size());
	m_Entities.push_back(newEntity);
	return newEntity.id;
}

void EntityWorld::RemoveEntity(uint32_t entityID)
{
	// prevent double deletion 
	auto* relPool = GetComponentPool<RelationshipComponent>();
	if (!relPool || !relPool->Contains(entityID)) return;

	// check for relationships
	RelationshipComponent relRef = relPool->Get(entityID);

	// recursively remove all of its children
	uint32_t curChild = relRef.firstChild;
	while (curChild != NULL_ENTITY)
	{
		if (relPool->Contains(curChild))
		{
			uint32_t nextChild = relPool->Get(curChild).nextSibling;
			RemoveEntity(curChild);
			curChild = nextChild;
		}
		else
		{
			curChild = NULL_ENTITY;
		}
	}

	// If child entity, unlink from parent
	if (relRef.parent != NULL_ENTITY && relPool->Contains(relRef.parent))
	{
		auto& parentRel = relPool->Get(relRef.parent);

		// update parent head if first child
		if (parentRel.firstChild == entityID)
		{
			parentRel.firstChild = relRef.nextSibling;
		}
	}

	if (relRef.prevSibling != NULL_ENTITY && relPool->Contains(relRef.prevSibling))
	{
		relPool->Get(relRef.prevSibling).nextSibling = relRef.nextSibling;
	}

	if (relRef.nextSibling != NULL_ENTITY && relPool->Contains(relRef.nextSibling))
	{
		relPool->Get(relRef.nextSibling).prevSibling = relRef.prevSibling;
	}

	// remove all components associated with this entity
	for (ISparseSet* pool : m_ComponentPools)
	{
		if (pool)
		{
			pool->Remove(entityID);
		}
	}
}

void EntityWorld::ClearAllEntities()
{
	for (ISparseSet* pool : m_ComponentPools)
	{
		if (pool) pool->Clear();
	}

	m_Entities.clear();
}
