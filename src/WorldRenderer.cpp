#include "WorldRenderer.h"
#include "BlockLibrary.h"
#include <glm/glm.hpp>
#include <iostream>

using namespace Craft;


void WorldRenderer::GenerateMesh(std::vector<Magma::Vertex>& vertices, std::vector<uint32_t>& indices, Chunk* chunk, glm::ivec2 chunkPos)
{
	BlockID* blocks = chunk->blocks;
	uint32_t indexOffset = 0;

	for (int i = 0; i < CHUNK_VOLUME; i++)
	{
		// referenced RGL
		
		if (blocks[i] != 0) // if this block isn't air, render
		{
			const Craft::BlockData& curBlockData = Craft::BlockLibrary::GetBlockData(blocks[i]);

			float x = (chunkPos.x * Craft::CHUNK_WIDTH) + chunk->GetBlockX(i);
			float y = chunk->GetBlockY(i);
			float z = (chunkPos.y * Craft::CHUNK_WIDTH) + chunk->GetBlockZ(i);
			glm::vec2 uv;

			// index = x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH);
			// mesh class expects pos, normals, texcoords, tangents, and bitangents

			// EAST FACE | EAST +X
			if (Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::EAST)
			).isTransparent)
			{
				Magma::Vertex v1, v2, v3, v4;
				uv = BlockLibrary::GetTexCoords(curBlockData.textureEast);

				// top left
				v1.Position = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
				v1.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
				v1.TexCoords = uv;
				v1.Tangent = glm::vec3(0.0f);
				v1.Bitangent = glm::vec3(0.0f);

				// top right
				v2.Position = glm::vec3(x + 0.5f, y + 0.5f, z - 0.5f);
				v2.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
				v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
				v2.Tangent = glm::vec3(0.0f);
				v2.Bitangent = glm::vec3(0.0f);

				// bottom right
				v3.Position = glm::vec3(x + 0.5f, y - 0.5f, z - 0.5f);
				v3.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
				v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
				v3.Tangent = glm::vec3(0.0f);
				v3.Bitangent = glm::vec3(0.0f);

				// bottom left
				v4.Position = glm::vec3(x + 0.5f, y - 0.5f, z + 0.5f);
				v4.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
				v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
				v4.Tangent = glm::vec3(0.0f);
				v4.Bitangent = glm::vec3(0.0f);

				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v3);
				vertices.push_back(v4);

				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset + 1);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 3);

				indexOffset += 4;
			}

			// WEST FACE | WEST -X
			if (Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::WEST)
			).isTransparent)
			{
				Magma::Vertex v1, v2, v3, v4;
				uv = BlockLibrary::GetTexCoords(curBlockData.textureWest);

				// top left
				v1.Position = glm::vec3(x - 0.5f, y + 0.5f, z - 0.5f);
				v1.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
				v1.TexCoords = uv;
				v1.Tangent = glm::vec3(0.0f);
				v1.Bitangent = glm::vec3(0.0f);

				// top right
				v2.Position = glm::vec3(x - 0.5f, y + 0.5f, z + 0.5f);
				v2.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
				v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
				v2.Tangent = glm::vec3(0.0f);
				v2.Bitangent = glm::vec3(0.0f);

				// bottom right
				v3.Position = glm::vec3(x - 0.5f, y - 0.5f, z + 0.5f);
				v3.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
				v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
				v3.Tangent = glm::vec3(0.0f);
				v3.Bitangent = glm::vec3(0.0f);

				// bottom left
				v4.Position = glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f);
				v4.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
				v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
				v4.Tangent = glm::vec3(0.0f);
				v4.Bitangent = glm::vec3(0.0f);

				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v3);
				vertices.push_back(v4);

				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset + 1);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 3);

				indexOffset += 4;
			}

			
			// TOP FACE | UP +Y
			if (Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::UP)
			).isTransparent)
			{
				Magma::Vertex v1, v2, v3, v4;
				uv = BlockLibrary::GetTexCoords(curBlockData.textureTop);

				// top left
				v1.Position = glm::vec3(x - 0.5f, y + 0.5f, z - 0.5f);
				v1.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v1.TexCoords = uv;
				v1.Tangent = glm::vec3(0.0f);
				v1.Bitangent = glm::vec3(0.0f);

				// top right
				v2.Position = glm::vec3(x + 0.5f, y + 0.5f, z - 0.5f);
				v2.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y); 
				v2.Tangent = glm::vec3(0.0f);
				v2.Bitangent = glm::vec3(0.0f);

				// bottom right
				v3.Position = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
				v3.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
				v3.Tangent = glm::vec3(0.0f);
				v3.Bitangent = glm::vec3(0.0f);

				// bottom left
				v4.Position = glm::vec3(x - 0.5f, y + 0.5f, z + 0.5f);
				v4.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
				v4.Tangent = glm::vec3(0.0f);
				v4.Bitangent = glm::vec3(0.0f);

				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v3);
				vertices.push_back(v4);

				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset + 1);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 3);

				indexOffset += 4;
			}

			// BOTTOM FACE | DOWN -Y
			if (Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::DOWN)
			).isTransparent)
			{
				Magma::Vertex v1, v2, v3, v4;
				uv = BlockLibrary::GetTexCoords(curBlockData.textureBottom);

				// top left
				v1.Position = glm::vec3(x - 0.5f, y - 0.5f, z + 0.5f);
				v1.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
				v1.TexCoords = uv;
				v1.Tangent = glm::vec3(0.0f);
				v1.Bitangent = glm::vec3(0.0f);

				// top right
				v2.Position = glm::vec3(x + 0.5f, y - 0.5f, z + 0.5f);
				v2.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
				v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
				v2.Tangent = glm::vec3(0.0f);
				v2.Bitangent = glm::vec3(0.0f);

				// bottom right
				v3.Position = glm::vec3(x + 0.5f, y - 0.5f, z - 0.5f);
				v3.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
				v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
				v3.Tangent = glm::vec3(0.0f);
				v3.Bitangent = glm::vec3(0.0f);

				// bottom left
				v4.Position = glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f);
				v4.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
				v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
				v4.Tangent = glm::vec3(0.0f);
				v4.Bitangent = glm::vec3(0.0f);

				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v3);
				vertices.push_back(v4);

				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset + 1);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 3);

				indexOffset += 4;
			}

			// SOUTH FACE | SOUTH +Z
			if (Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::SOUTH)
			).isTransparent)
			{
				Magma::Vertex v1, v2, v3, v4;
				uv = BlockLibrary::GetTexCoords(curBlockData.textureSouth);

				// top left
				v1.Position = glm::vec3(x - 0.5f, y + 0.5f, z + 0.5f);
				v1.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
				v1.TexCoords = uv;
				v1.Tangent = glm::vec3(0.0f);
				v1.Bitangent = glm::vec3(0.0f);

				// top right
				v2.Position = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
				v2.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
				v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
				v2.Tangent = glm::vec3(0.0f);
				v2.Bitangent = glm::vec3(0.0f);

				// bottom right
				v3.Position = glm::vec3(x + 0.5f, y - 0.5f, z + 0.5f);
				v3.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
				v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
				v3.Tangent = glm::vec3(0.0f);
				v3.Bitangent = glm::vec3(0.0f);

				// bottom left
				v4.Position = glm::vec3(x - 0.5f, y - 0.5f, z + 0.5f);
				v4.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
				v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
				v4.Tangent = glm::vec3(0.0f);
				v4.Bitangent = glm::vec3(0.0f);

				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v3);
				vertices.push_back(v4);

				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset + 1);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 3);

				indexOffset += 4;
			}

			// NORTH FACE | NORTH -Z
			if (Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::NORTH)
			).isTransparent)
			{
				Magma::Vertex v1, v2, v3, v4;
				uv = BlockLibrary::GetTexCoords(curBlockData.textureNorth);

				// top left
				v1.Position = glm::vec3(x + 0.5f, y + 0.5f, z - 0.5f);
				v1.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
				v1.TexCoords = uv;
				v1.Tangent = glm::vec3(0.0f);
				v1.Bitangent = glm::vec3(0.0f);

				// top right
				v2.Position = glm::vec3(x - 0.5f, y + 0.5f, z - 0.5f);
				v2.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
				v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
				v2.Tangent = glm::vec3(0.0f);
				v2.Bitangent = glm::vec3(0.0f);

				// bottom right
				v3.Position = glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f);
				v3.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
				v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
				v3.Tangent = glm::vec3(0.0f);
				v3.Bitangent = glm::vec3(0.0f);

				// bottom left
				v4.Position = glm::vec3(x + 0.5f, y - 0.5f, z - 0.5f);
				v4.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
				v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
				v4.Tangent = glm::vec3(0.0f);
				v4.Bitangent = glm::vec3(0.0f);

				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v3);
				vertices.push_back(v4);

				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset + 1);
				indices.push_back(indexOffset + 2);
				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 3);

				indexOffset += 4;
			}
			
		}
	}
}

bool WorldRenderer::AddMeshToDrawPool(std::unique_ptr<Magma::Mesh> mesh, glm::ivec2 chunkPos)
{
	// if already in draw pool
	if (m_DrawPool.contains(chunkPos)) return false;

	m_DrawPool[chunkPos] = std::move(mesh);
	return true;
}

void WorldRenderer::RemoveFromDrawPool(glm::ivec2 chunkPos)
{
	m_DrawPool.erase(chunkPos);
}

void WorldRenderer::DrawWorld()
{
	if (m_DrawPool.empty()) return;

	// Calling Mesh->Draw()
	for (const auto& pair : m_DrawPool)
	{
		pair.second->Draw();
	}
}