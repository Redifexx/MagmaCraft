#pragma once

#include <Datatypes/EntityWorld.h>
#include <Datatypes/Components/TransformComponent.h>
#include <Datatypes/Components/RelationshipComponent.h>
#include <Datatypes/Components/ModelComponent.h>
#include <Datatypes/Components/CameraComponent.h>
#include <Datatypes/Components/HealthComponent.h>

namespace Craft
{
	class ScriptableEntity
	{
		public:
			virtual ~ScriptableEntity() = default;

			EntityID entityID;
			EntityWorld* world = nullptr;

			template <typename T>
			T& GetComponent() { return world->GetComponent<T>(entityID); }

			virtual void OnAttach() {}
			virtual void OnUpdate(float dt) {}
			virtual void OnDestroy() {}
	};
}