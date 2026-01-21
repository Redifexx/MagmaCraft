#include "WorldRenderer.h"
#include "BlockLibrary.h"
#include <glm/glm.hpp>

using namespace Craft;

WorldRenderer::WorldRenderer()
{
}

void WorldRenderer::RenderChunk(Chunk* chunk, glm::ivec2 chunkPos)
{
	std::unique_ptr<Magma::Mesh> mesh = std::make_unique<Magma::Mesh>();
	GenerateMesh(*mesh, chunk, chunkPos);

}

void WorldRenderer::GenerateMesh(Magma::Mesh& mesh, Chunk* chunk, glm::ivec2 chunkPos)
{
	BlockID* blocks = chunk->blocks;

	for (int i = 0; i < CHUNK_VOLUME; i++)
	{
		// reference rgl
		
		if (blocks[i] != 0) // if this block isn't air, render
		{
			const Craft::BlockData& curBlockData = Craft::BlockLibrary::GetBlockData(blocks[i]);

			// index = x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH);
			// mesh class expects pos, normals, texcoords, tangents, and bitangents
			
			// TOPFACE
			if (Craft::BlockLibrary::GetBlockData(chunk->GetBlockNeighbor(i, Direction::UP)).isTransparent)
			{
				Magma::Vertex v1, v2, v3, v4;
				v1.Position = glm::vec3(chunkPos.x + chunk->GetBlockX(i) + -0.5f, chunk->GetBlockY(i) + 0.5f, chunkPos.y + chunk->GetBlockZ(i) + 0.5f);
				v1.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v1.TexCoords = glm::vec2()

			}
			
		}
	}
}
