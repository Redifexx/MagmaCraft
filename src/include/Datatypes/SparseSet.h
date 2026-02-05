#pragma once

#include <cstdint>
#include <vector>
#include <limits> 

// key to an almost pure ecs approach
// sparse set implementation
// keeps two vectors: a sparse one for holding an index, and a packed one with component data

namespace Craft
{
	using EntityID = uint32_t;

	// this will represent a null entity since -1 isn't possible
	const uint32_t NULL_ENTITY = (std::numeric_limits<uint32_t>::max)();

	// Interface so I can store different sparse sets in one map
	class ISparseSet
	{
		public:
			virtual ~ISparseSet() = default;
			virtual void Remove(EntityID entity) = 0;
			virtual bool Contains(EntityID entity) const = 0;
			virtual void Clear() = 0;
			virtual const std::vector<EntityID>& GetAllEntities() const = 0;
	};

	template <typename T>
	class SparseSet : public ISparseSet
	{
		public:
			T& Add(EntityID entity, const T& component)
			{
				// allocate space in sparse array if needed
				if (entity >= m_Sparse.size())
				{
					m_Sparse.resize(entity + 1, NULL_ENTITY);
				}

				// add component to dense array
				m_Components.push_back(component);
				m_DenseToEntity.push_back(entity);

				// link sprase array to dense array
				uint32_t denseIndex = (uint32_t)m_Components.size() - 1;
				m_Sparse[entity] = denseIndex;

				return m_Components.back();
			}

			void Remove(EntityID entity) override
			{
				if (!Contains(entity)) return;

				uint32_t indexToRemove = m_Sparse[entity];
				uint32_t lastIndex = (uint32_t)m_Components.size() - 1;

				// swap the component to remove with the last component
				if (indexToRemove != lastIndex)
				{
					// get the entity id of the last component
					EntityID lastEntity = m_DenseToEntity[lastIndex];

					// move last component to index to remove
					m_Components[indexToRemove] = m_Components[lastIndex];
					m_DenseToEntity[indexToRemove] = lastEntity;

					// update sparse array for last entity
					m_Sparse[lastEntity] = indexToRemove;
				}

				// remove last component
				m_Components.pop_back();
				m_DenseToEntity.pop_back();

				// mark entity as removed in sparse array
				m_Sparse[entity] = NULL_ENTITY;
			}

			T& Get(EntityID entity)
			{
				//assert(Contains(entity) && "Entity doesn't have this component.");
				return m_Components[m_Sparse[entity]];
			}

			bool Contains(EntityID entity) const override
			{
				return entity < m_Sparse.size() && m_Sparse[entity] != NULL_ENTITY;
			}

			void Clear() override
			{
				m_Components.clear();
				m_DenseToEntity.clear();
				m_Sparse.clear();
			}

			// for iteration
			const std::vector<T>& GetAllComponents() const
			{
				return m_Components;
			}

			const std::vector<EntityID>& GetAllEntities() const override
			{
				return m_DenseToEntity;
			}

		private:	
			std::vector<T> m_Components;
			std::vector<EntityID> m_DenseToEntity; // maps component index to entity id
			std::vector<uint32_t> m_Sparse;
	};
}