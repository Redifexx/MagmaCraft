#pragma once

#include <cstdint>
#include <limits>

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

	using EntityID = uint32_t;

	// this will represent a null entity since -1 isn't possible
	const uint32_t NULL_ENTITY = std::numeric_limits<uint32_t>::max();

	class EntityWorld
	{
		public:
			
			// make template for adding and removing components

		private:

			// sparse sets for components
			
	};
}
