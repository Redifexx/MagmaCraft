#pragma once

namespace Craft
{
	struct RelationshipComponent
	{
		EntityID parent = NULL_ENTITY;
		EntityID firstChild = NULL_ENTITY;
		EntityID prevSibling = NULL_ENTITY;
		EntityID nextSibling = NULL_ENTITY;
	};
}