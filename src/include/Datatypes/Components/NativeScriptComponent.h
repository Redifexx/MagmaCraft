#pragma once

#include <Datatypes/ScriptableEntity.h>
#include <functional>

// Component that enables scripts to be attached to entities
namespace Craft
{
	struct NativeScriptComponent
	{
		ScriptableEntity* instance = nullptr;

		// function pointers to create/destroy script instances
		std::function<ScriptableEntity* ()> instantiateScript;
		std::function<void(NativeScriptComponent*)> destroyScript;

		template<typename T>
		void Bind()
		{
			instantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			destroyScript = [](NativeScriptComponent* nsc)
			{
				delete nsc->instance;
				nsc->instance = nullptr;
			};
		}
	};
}