#include "Datatypes/EntityWorld.h"
#include <iostream>

using namespace Craft;


inline uint32_t Craft::GetNextComponentID()
{
	static uint32_t id = 0;
	return id++;
}

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

// not sure if this is the best way to assign entity IDs
uint32_t EntityWorld::AddEntity()
{
	if (!m_FreeEntityIDs.empty())
	{
		uint32_t id = m_FreeEntityIDs.back();
		m_FreeEntityIDs.pop_back();

		// ensure the id isn't occupied
		for (ISparseSet* pool : m_ComponentPools)
		{
			if (pool) pool->Remove(id);
		}

		return id;
	}
	return m_NextEntityID++;
}

void EntityWorld::RemoveEntity(uint32_t entityID)
{
	// no invalid ids
	if (entityID >= m_NextEntityID) return;

	// check if already in list
	for (uint32_t freeEID : m_FreeEntityIDs)
	{
		if (freeEID == entityID) return; // try to make O(1) replacmenet
	}

	auto* relPool = GetComponentPool<RelationshipComponent>();
	if (relPool && relPool->Contains(entityID))
	{
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
	}

	// remove all components associated with this entity
	for (ISparseSet* pool : m_ComponentPools)
	{
		if (pool)
		{
			pool->Remove(entityID);
		}
	}

	// reuse id
	m_FreeEntityIDs.push_back(entityID);
}

void EntityWorld::ClearAllEntities()
{
	std::cout << "Clearing all entities..." << std::endl;
	for (ISparseSet* pool : m_ComponentPools)
	{
		if (pool) pool->Clear();
	}

	m_PlayerIDEntityMap.clear();
	m_FreeEntityIDs.clear();
	m_LocalPlayerID = NULL_ENTITY;
	m_NextEntityID = 0;
	std::cout << "All entities cleared." << std::endl;
}

bool EntityWorld::HasEntityID(uint32_t entityID)
{
	if (entityID >= m_NextEntityID) return false;

	for (uint32_t deadID : m_FreeEntityIDs)
	{
		if (deadID == entityID) return false;
	}

	return true;
}
