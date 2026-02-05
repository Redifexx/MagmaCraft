#pragma once

#include <Core/Mesh.h>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>

// turns external models into multiple meshes
namespace Magma
{
	class Model
	{
		public:
			Model(const std::string& filepath);

			Model(Model&&) noexcept = default;
			Model& operator=(Model&&) noexcept = default;

			Model(const Model&) = delete;
			Model& operator=(const Model&) = delete;

			void Draw() const;

		private:
			std::vector<Mesh> m_Meshes;
			std::string m_Directory;

			void ProcessNode(aiNode* node, const aiScene* scene);
			Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	};
}