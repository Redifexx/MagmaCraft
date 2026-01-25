#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

namespace Magma
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
	};

	class Mesh
	{
		public:
			Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

			void Draw() const;
		private:
			unsigned int m_VAO, m_VBO, m_EBO;
			std::vector<Vertex> m_Vertices;
			std::vector<unsigned int> m_Indices;
	};
}