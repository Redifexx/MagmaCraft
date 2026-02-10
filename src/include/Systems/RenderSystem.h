#pragma once
#include "Core/ShaderProgram.h"
#include "Datatypes/EntityWorld.h"
#include "WorldStreamer.h"
#include <glad/glad.h>
#include <SDL3/SDL.h>

// Brings the world rendering and entity rendering together
// need to improve the texture/shader implementation, might later make resource manager
namespace Craft
{
	class RenderSystem
	{
		public:
			void Render(EntityWorld& world, const Magma::ShaderProgram& shaderProgram, WorldStreamer* worldStreamer, SDL_Window* window, bool shadowPass);
		private:
			void DrawEntities(EntityWorld& world, const Magma::ShaderProgram& shaderProgram, WorldStreamer* worldStreamer, bool shadowPass);
			void SetupShaderUniforms(EntityWorld& world, const Magma::ShaderProgram& shaderProgram);
	};
}