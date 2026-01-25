#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

namespace Magma
{
	class ShaderProgram
	{
		public:
			ShaderProgram();
			~ShaderProgram();
			void AttachShader(const Shader& shader);
			bool Link();
			void Use() const;

			void SetUniform(const std::string& name, int value) const;
			void SetUniform(const std::string& name, float value) const;
			void SetUniform(const std::string& name, const glm::mat4& mat) const;
		private:
			GLuint m_ID;
	};
}