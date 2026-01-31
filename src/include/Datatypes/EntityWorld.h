#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include "Datatypes/SparseSet.h"
#include <Datatypes/Components/TransformComponent.h>
#include <Datatypes/Components/RelationshipComponent.h>
#include <Datatypes/Components/ModelComponent.h>
#include <Datatypes/Components/CameraComponent.h>
#include <Datatypes/Components/HealthComponent.h>

/*
ECS RULES
-- Entities just hold an ID
-- Components don't share a base class, instead they are simple structs with data
-- Split Component Data into a dense array of components and sparse array of indexes
-- Compoenets may have helper functions to only modify local data
-- For components, stats are objects, resources are pointers, and entities are ids

*/
namespace Craft
{
	// just an index
	struct Entity
	{
		uint32_t id;
	};

	// helpers for vector of sparse sets, turns components into IDs at runtime
	inline uint32_t GetNextComponentID()
	{
		static uint32_t id = 0;
		return id++;
	}

	template <typename T>
	uint32_t GetComponentID()
	{
		static uint32_t typeID = GetNextComponentID();
		return typeID;
	}

	class EntityWorld
	{
		public:
			template <typename T>
			SparseSet<T>* GetComponentPool()
			{
				uint32_t id = GetComponentID<T>();

				// increase vector size if needed
				if (id >= m_ComponentPools.size())
				{
					m_ComponentPools.resize(id + 1, nullptr);
				}

				// create new sparse set if it doesn't exist
				if (!m_ComponentPools[id])
				{
					m_ComponentPools[id] = new SparseSet<T>();
				}

				// cast it back to specific type
				return static_cast<SparseSet<T>*>(m_ComponentPools[id]);
			}

			template <typename T>
			T& AddComponent(uint32_t entityID, T component)
			{
				return GetComponentPool<T>()->Add(entityID, component);
			}

			template <typename T>
			void RemoveComponent(uint32_t entityID)
			{
				GetComponentPool<T>()->Remove(entityID);
			}

			template <typename T>
			T& GetComponent(uint32_t entityID)
			{
				return GetComponentPool<T>()->Get(entityID);
			}

			// provide a view of entities that have all specified components
			// variadic templates - can take in any number of arguments/component types
			// very nice, very useful
			template<typename... ComponentTypes>
			std::vector<EntityID> View()
			{
				std::vector<EntityID> result;

				// get all the pools as the base ISparseSet
				std::vector<ISparseSet*> pools = { GetComponentPool<ComponentTypes>()... };

				// return empty if pool is invalid
				for (auto* pool : pools)
				{
					if (!pool || pool->GetAllEntities().empty())
					{
						return result;
					}
				}

				// start with the smallest pool to minimize iterations
				ISparseSet* smallestPool = pools[0];
				size_t smallestSize = smallestPool->GetAllEntities().size();

				for (size_t i = 1; i < pools.size(); ++i)
				{
					if (pools[i]->GetAllEntities().size() < smallestSize)
					{
						smallestPool = pools[i];
						smallestSize = pools[i]->GetAllEntities().size();
					}
				}

				// iterate over entities in smallest pool
				const std::vector<EntityID>& entities = smallestPool->GetAllEntities();
				result.reserve(smallestSize);

				for (EntityID entity : entities)
				{
					bool hasAll = true;

					// check if it has the other entities
					for (auto* pool : pools)
					{
						if (pool == smallestPool) continue;

						if (!pool->Contains(entity))
						{
							hasAll = false;
							break;
						}
					}

					if (hasAll)
					{
						result.push_back(entity);
					}
				}
				return result;
			}

			EntityWorld();
			~EntityWorld();

			// add entity function / remove entity function
			uint32_t AddEntity();
			void RemoveEntity(uint32_t entityID);

			void ClearAllEntities();
			
		private:
			std::vector<Entity> m_Entities;
			std::vector<ISparseSet*> m_ComponentPools; // one per component type

	};
}
