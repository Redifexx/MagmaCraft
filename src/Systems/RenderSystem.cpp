#include "RenderSystem.h"
#include <Datatypes/Components/TransformComponent.h>
#include <Datatypes/Components/ModelComponent.h>
#include <Datatypes/Components/CameraComponent.h>
#include <Core/Model.h>

using namespace Craft;

void RenderSystem::Render(EntityWorld& world, const Magma::ShaderProgram& shaderProgram, const WorldStreamer& worldStreamer)
{
	shaderProgram.Use();

	SetupShaderUniforms(world, shaderProgram);	

	if (worldStreamer)
	{
		worldStreamer.GetWorldRenderer()->DrawWorld();
	}

	DrawEntities(world, shaderProgram);
}

void RenderSystem::DrawEntities(EntityWorld& world, const Magma::ShaderProgram& shaderProgram)
{
	SparseSet<TransformComponent>* transformPool = world.GetComponentPool<TransformComponent>();
	SparseSet<ModelComponent>* modelPool = world.GetComponentPool<ModelComponent>();

	for (auto entity : modelPool->GetAllEntities())
	{
		if (!transformPool->Contains(entity)) return;

		// gets component references
		auto& modelRef = modelPool->Get(entity);
		auto& transformRef = transformPool->Get(entity);

		shaderProgram.SetUniform("u_Model", transformRef.worldMatrix);

		if (modelRef.model)
		{
			modelRef.model->Draw();
		}
	}
}

void RenderSystem::SetupShaderUniforms(EntityWorld& world, const Magma::ShaderProgram& shaderProgram)
{
	SparseSet<TransformComponent>* transformPool = world.GetComponentPool<TransformComponent>();
	SparseSet<CameraComponent>* cameraPool = world.GetComponentPool<CameraComponent>();

	// find primary camera
	const std::vector<uint32_t>& entities = cameraPool->GetAllEntities();
	for (uint32_t entity : entities)
	{
		// gets cam reference
		auto& camRef = cameraPool->Get(entity);

		if (camRef.isPrimary)
		{
			glm::mat4 viewProj = camRef.projectionMatrix * camRef.viewMatrix;

			shaderProgram.SetUniform("u_ViewProjection", viewProj);

			if (transformPool->Contains(entity))
			{
				shaderProgram.SetUniform("u_CameraPosition", transformPool->Get(entity).worldMatrix[3]);
			}
			break;
		}
	}
}