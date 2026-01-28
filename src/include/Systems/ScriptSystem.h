#pragma once
#include <Datatypes/EntityWorld.h>
#include <Datatypes/Components/NativeScriptComponent.h>

namespace Craft
{
	class ScriptSystem
	{
		public:
			void Update(EntityWorld& world, float dt);
	};
}