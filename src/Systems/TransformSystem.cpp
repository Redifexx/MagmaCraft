#include <Systems/TransformSystem.h>
#include <Datatypes/EntityWorld.h>

using namespace Craft;

void TransformSystem::Update(EntityWorld& world)
{
	// get all entites with transform and relationship components
	SparseSet<TransformComponent>* transformPool = world.GetComponentPool<TransformComponent>();
	SparseSet<RelationshipComponent>* relationshipPool = world.GetComponentPool<RelationshipComponent>();

	if (!transformPool || !relationshipPool) return;

	// iterate through root entities using relationship pool
	const std::vector<uint32_t>& entities = relationshipPool->GetAllEntities();
	for (uint32_t entity : entities)
	{
		if (transformPool->Contains(entity))
		{
			auto& relRef = relationshipPool->Get(entity);
			if (relRef.parent == NULL_ENTITY)
			{
				UpdateWorldMatrix(world, entity, glm::mat4(1.0f), false);
			}
		}
	}
}

void TransformSystem::UpdateWorldMatrix(EntityWorld& world, uint32_t entityID, const glm::mat4& parentMatrix, bool isParentDirty)
{
	TransformComponent& transform = world.GetComponent<TransformComponent>(entityID);
	RelationshipComponent& relationship = world.GetComponent<RelationshipComponent>(entityID);

	if (transform.isDirty)
	{
		// recalculate local matrix
		transform.localMatrix = glm::translate(glm::mat4(1.0f), transform.localPosition) *
			glm::mat4_cast(transform.localRotation) *
			glm::scale(glm::mat4(1.0f), transform.localScale);
	}

	bool isDirty = transform.isDirty || isParentDirty;

	if (isDirty)
	{
		// recalculate world matrix
		transform.worldMatrix = parentMatrix * transform.localMatrix;

		transform.isDirty = false;
	}

	// if no child
	if (relationship.firstChild == NULL_ENTITY) return;

	// if so, update children
	uint32_t curChildID = relationship.firstChild;

	while (curChildID != NULL_ENTITY)
	{
		UpdateWorldMatrix(world, curChildID, transform.worldMatrix, isDirty);
		RelationshipComponent& childRel = world.GetComponent<RelationshipComponent>(curChildID);
		curChildID = childRel.nextSibling;
	}
}