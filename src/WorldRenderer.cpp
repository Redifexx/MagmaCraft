#include "WorldRenderer.h"
#include "BlockLibrary.h"
#include <glm/glm.hpp>

using namespace Craft;

WorldRenderer::WorldRenderer()
{
}

void WorldRenderer::RenderChunk(Chunk* chunk, glm::ivec2 chunkPos)
{
	std::unique_ptr<Magma::Mesh> mesh;
	GenerateMesh(*mesh, chunk, chunkPos);

}

void WorldRenderer::GenerateMesh(Magma::Mesh& mesh, Chunk* chunk, glm::ivec2 chunkPos)
{
	BlockID* blocks = chunk->blocks;
	std::vector<Magma::Vertex> vertices;
	std::vector<uint32_t> indices;
	uint32_t indexOffset = 0;

	for (int i = 0; i < CHUNK_VOLUME; i++)
	{
		// reference rgl
		
		if (blocks[i] != 0) // if this block isn't air, render
		{
			const Craft::BlockData& curBlockData = Craft::BlockLibrary::GetBlockData(blocks[i]);

			float x = chunkPos.x + chunk->GetBlockX(i);
			float y = chunk->GetBlockY(i);
			float z = chunkPos.y + chunk->GetBlockZ(i);
			glm::vec2 uv = BlockLibrary::GetTexCoords(curBlockData.textureTop);

			// index = x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH);
			// mesh class expects pos, normals, texcoords, tangents, and bitangents
			
			// TOPFACE
			if (Craft::BlockLibrary::GetBlockData(chunk->GetBlockNeighbor(i, Direction::UP)).isTransparent)
			{
				Magma::Vertex v1, v2, v3, v4;

				// top left
				v1.Position = glm::vec3(x + -0.5f, y + 0.5f, z + 0.5f);
				v1.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v1.TexCoords = uv;
				v1.Tangent = glm::vec3(0.0f);
				v1.Bitangent = glm::vec3(0.0f);

				// top right
				v2.Position = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
				v2.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y); 
				v2.Tangent = glm::vec3(0.0f);
				v2.Bitangent = glm::vec3(0.0f);

				// bottom right
				v3.Position = glm::vec3(x + 0.5f, y + 0.5f, z - 0.5f);
				v3.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
				v3.Tangent = glm::vec3(0.0f);
				v3.Bitangent = glm::vec3(0.0f);

				// bottom left
				v4.Position = glm::vec3(x - 0.5f, y + 0.5f, z - 0.5f);
				v4.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
				v4.Tangent = glm::vec3(0.0f);
				v4.Bitangent = glm::vec3(0.0f);

				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v3);
				vertices.push_back(v4);

				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 1);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset + 3);
				indices.push_back(indexOffset);

				indexOffset += 4;
			}
			
		}
	}
}
