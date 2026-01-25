#pragma once

#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace Magma
{
	class Shader
	{
		public:
			Shader(const std::string& filepath, GLenum type);
			~Shader();
			bool Compile();

			GLuint GetID() const { return m_ID; }
		private:
			GLuint m_ID;
			GLenum m_Type;
	};
}