#include <Systems/ScriptSystem.h>

using namespace Craft;

void ScriptSystem::Update(EntityWorld& world, float dt)
{
	// iterate through every entity with a NativeScriptComponent
	SparseSet<NativeScriptComponent>* scriptPool = world.GetComponentPool<NativeScriptComponent>();
	if (!scriptPool) return;

	const std::vector<uint32_t>& entities = scriptPool->GetAllEntities();
	for (uint32_t entity : entities)
	{
		NativeScriptComponent& nsc = scriptPool->Get(entity);
		if (!nsc.instance)
		{
			// instantiate script
			nsc.instance = nsc.instantiateScript();
			nsc.instance->entityID = entity;
			nsc.instance->world = &world;
			nsc.instance->OnAttach();
		}

		nsc.instance->OnUpdate(dt);
	}
}