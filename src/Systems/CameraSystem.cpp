#include <Systems/CameraSystem.h>

using namespace Craft;

CameraSystem::CameraSystem()
{
	m_Camera = std::make_unique<Magma::Camera>();
}

void CameraSystem::Update(EntityWorld& world)
{
	// get all entites with transform and relationship components
	SparseSet<TransformComponent>* transformPool = world.GetComponentPool<TransformComponent>();
	SparseSet<CameraComponent>* cameraPool = world.GetComponentPool<CameraComponent>();

	if (!transformPool || !cameraPool) return;


	// iterate through root entities using camera pool
	const std::vector<uint32_t>& entities = cameraPool->GetAllEntities();
	for (uint32_t entity : entities)
	{
		if (transformPool->Contains(entity))
		{
			auto& transRef = transformPool->Get(entity);
			auto& camRef = cameraPool->Get(entity);

			const glm::mat4& worldMatrix = transRef.worldMatrix;

			// extract camera pos from matrix
			glm::vec3 worldPos = worldMatrix[3];
			glm::vec3 worldFront = -glm::vec3(worldMatrix[2]);
			glm::vec3 worldUp = glm::vec3(worldMatrix[1]);

			m_Camera->SetPosition(worldPos);
			m_Camera->SetOrientation(worldFront, worldUp);
		}
	}
}