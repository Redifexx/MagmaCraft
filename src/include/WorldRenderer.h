#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "Mesh.h"
#include <memory>
#include <cstdint>

// Receives chunks from server
// Caches a number of chunks around the player
// Renders cached chunks
namespace Craft
{
	struct ChunkMesh
	{
		std::unique_ptr<Magma::Mesh> mesh;
		int chunkX = 0;
		int chunkZ = 0;
	};
	class WorldRenderer
	{
		public:
			WorldRenderer();
			void RenderChunk(Chunk* chunk);
			void GenerateMesh(Magma::Mesh& mesh, BlockID* blocks);

		private:
			std::vector<ChunkMesh> m_MeshPool;
	};
}