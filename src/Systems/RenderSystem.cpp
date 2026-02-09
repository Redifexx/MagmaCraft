#include "Systems/RenderSystem.h"
#include <Datatypes/Components/TransformComponent.h>
#include <Datatypes/Components/ModelComponent.h>
#include <Datatypes/Components/CameraComponent.h>
#include <Datatypes/Components/PlayerComponent.h>
#include <Core/Model.h>
#include <glm/glm.hpp>
#include "Core/Texture.h"

using namespace Craft;

// renders all
void RenderSystem::Render(EntityWorld& world, const Magma::ShaderProgram& shaderProgram, WorldStreamer* worldStreamer, SDL_Window* window)
{
	shaderProgram.Use();

	SetupShaderUniforms(world, shaderProgram);	

	shaderProgram.SetUniform("u_Model", glm::mat4(1.0f));
	glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat4(1.0f)));
	shaderProgram.SetUniform("u_NormalMatrix", (glm::mat3)normalMat);

	auto worldManager = worldStreamer->GetWorldManager().get();
	if (!worldManager) return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, worldManager->m_BlockAtlasTextureAlbedo ->GetID());
	shaderProgram.SetUniform("u_AlbedoTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, worldManager->m_BlockAtlasTextureNormal->GetID());
	shaderProgram.SetUniform("u_NormalTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, worldManager->m_BlockAtlasTextureASME->GetID());
	shaderProgram.SetUniform("u_ASMETexture", 2);

	if (worldStreamer)
	{
		worldStreamer->GetWorldRenderer()->DrawWorld();
	}

	DrawEntities(world, shaderProgram, worldStreamer);
}

// iterates through entities with a model and renders them
void RenderSystem::DrawEntities(EntityWorld& world, const Magma::ShaderProgram& shaderProgram, WorldStreamer* worldStreamer)
{
	SparseSet<PlayerComponent>* playerPool = world.GetComponentPool<PlayerComponent>();
	SparseSet<TransformComponent>* transformPool = world.GetComponentPool<TransformComponent>();
	SparseSet<ModelComponent>* modelPool = world.GetComponentPool<ModelComponent>();

	if (!transformPool || !modelPool) return;

	const std::vector<uint32_t>& entities = modelPool->GetAllEntities();

	//auto entities = world.View<TransformComponent, ModelComponent>(); broken for now

	glm::mat4 worldMatrix = glm::mat4(1.0f);

	for (auto entity : entities)
	{
		if (!transformPool->Contains(entity)) continue;

		// gets component references
		auto& modelRef = world.GetComponent<ModelComponent>(entity);
		auto& transformRef = world.GetComponent<TransformComponent>(entity);
		worldMatrix = transformRef.worldMatrix;


		shaderProgram.SetUniform("u_Model", worldMatrix);
		glm::mat3 normalMat = glm::transpose(glm::inverse(worldMatrix));
		shaderProgram.SetUniform("u_NormalMatrix", (glm::mat3)normalMat);

		if (playerPool->Contains(entity))
		{
			auto& playerRef = playerPool->Get(entity);

			if (playerRef.isLocalPlayer)
			{
				continue;
			}
			
			// UPDATE PLAYER TEXTURES
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, playerRef.skin_albedo->GetID());
			shaderProgram.SetUniform("u_AlbedoTexture", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, playerRef.skin_normal->GetID());
			shaderProgram.SetUniform("u_NormalTexture", 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, playerRef.skin_asme->GetID());
			shaderProgram.SetUniform("u_ASMETexture", 2);
		}
		else
		{
			auto worldManager = worldStreamer->GetWorldManager().get();
			if (!worldManager) return;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, worldManager->m_BlockAtlasTextureAlbedo->GetID());
			shaderProgram.SetUniform("u_AlbedoTexture", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, worldManager->m_BlockAtlasTextureNormal->GetID());
			shaderProgram.SetUniform("u_NormalTexture", 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, worldManager->m_BlockAtlasTextureASME->GetID());
			shaderProgram.SetUniform("u_ASMETexture", 2);
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
				// dont send cam pos until deferred rendering / view depending shading is added
				//shaderProgram.SetUniform("u_CameraPosition", glm::vec3(transformPool->Get(entity).worldMatrix[3]));
			}
			break;
		}
	}
}