#pragma once

#include <Datatypes/ScriptableEntity.h>
#include <functional>

namespace Craft
{
	struct NativeScriptComponent
	{
		std::shared_ptr<ScriptableEntity> instance = nullptr;

		// function pointers to create/destroy script instances
		std::function<ScriptableEntity*()> instantiateScript;
		std::function<void(NativeScriptComponent*)> destroyScript;

		template<typename T>
		void Bind()
		{
			instantiteScript = []() { return static_cast<ScriptableEntity*>(new T()); }
			destroyScript = [](NativeScriptComponent* nsc)
			{
				delete nsc->instance;
				nsc->instance = nullptr;
			};
		}
	}
}