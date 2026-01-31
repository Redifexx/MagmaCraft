#include "Systems/RenderSystem.h"
#include <Datatypes/Components/TransformComponent.h>
#include <Datatypes/Components/ModelComponent.h>
#include <Datatypes/Components/CameraComponent.h>
#include <Datatypes/Components/PlayerComponent.h>
#include <Core/Model.h>
#include <glm/glm.hpp>

using namespace Craft;

void RenderSystem::Render(EntityWorld& world, const Magma::ShaderProgram& shaderProgram, WorldStreamer* worldStreamer, SDL_Window* window)
{
	//clear screen
	int w_, h_;
	SDL_GetWindowSize(window, &w_, &h_);
	glViewport(0, 0, w_, h_);
	glClearColor(0.643f, 0.827f, 0.984f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	shaderProgram.Use();

	SetupShaderUniforms(world, shaderProgram);	

	shaderProgram.SetUniform("u_Model", glm::mat4(1.0f));

	if (worldStreamer)
	{
		worldStreamer->GetWorldRenderer()->DrawWorld();
	}

	DrawEntities(world, shaderProgram);
}

void RenderSystem::DrawEntities(EntityWorld& world, const Magma::ShaderProgram& shaderProgram)
{
	SparseSet<PlayerComponent>* playerPool = world.GetComponentPool<PlayerComponent>();

	auto entities = world.View<TransformComponent, ModelComponent>();

	glm::mat4 worldMatrix = glm::mat4(1.0f);

	for (auto entity : entities)
	{
	
		// gets component references
		auto& modelRef = world.GetComponent<ModelComponent>(entity);
		auto& transformRef = world.GetComponent<TransformComponent>(entity);
		worldMatrix = transformRef.worldMatrix;

		shaderProgram.SetUniform("u_Model", worldMatrix);

		if (playerPool->Contains(entity))
		{
			auto& playerRef = playerPool->Get(entity);
			if (playerRef.isLocalPlayer) return; // dont draw local player model
		}

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
				// dont send cam pos until defered rendering is added
				//shaderProgram.SetUniform("u_CameraPosition", glm::vec3(transformPool->Get(entity).worldMatrix[3]));
			}
			break;
		}
	}
}