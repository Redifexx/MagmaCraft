#pragma once
#include <Datatypes/EntityWorld.h>
#include <Datatypes/Components/NativeScriptComponent.h>

// updates all scripts :)
namespace Craft
{
	class ScriptSystem
	{
		public:
			void Update(EntityWorld& world, float dt);
	};
}