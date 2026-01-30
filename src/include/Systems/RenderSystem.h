#pragma once
#include "Core/ShaderProgram.h"
#include "Datatypes/EntityWorld.h"
#include "WorldStreamer.h"
#include <glad/glad.h>
#include <SDL3/SDL.h>

namespace Craft
{
	class RenderSystem
	{
		public:
			void Render(EntityWorld& world, const Magma::ShaderProgram& shaderProgram, WorldStreamer* worldStreamer, SDL_Window* window);
		private:
			void DrawEntities(EntityWorld& world, const Magma::ShaderProgram& shaderProgram);
			void SetupShaderUniforms(EntityWorld& world, const Magma::ShaderProgram& shaderProgram);
	};
}