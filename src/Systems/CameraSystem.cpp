#include <Systems/CameraSystem.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace Craft;

void CameraSystem::Update(EntityWorld& world)
{
	// get all entites with transform and camera components (shuold be 1)
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

			camRef.viewMatrix = glm::inverse(transRef.worldMatrix);

			if (camRef.cameraType == CameraType::PERSPECTIVE)
			{
				camRef.projectionMatrix = glm::perspective(
					glm::radians(camRef.FOV),
					camRef.aspectRatio,
					camRef.nearPlane,
					camRef.farPlane
				);
			}
			else
			{
				float orthoHeight = camRef.orthoSize;
				float orthoWidth = orthoHeight * camRef.aspectRatio;
				camRef.projectionMatrix = glm::ortho(
					-orthoWidth / 2.0f, orthoWidth / 2.0f,
					-orthoHeight / 2.0f, orthoHeight / 2.0f,
					camRef.nearPlane,
					camRef.farPlane
				);
			}
		}
	}
}