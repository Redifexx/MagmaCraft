#include "WorldRenderer.h"
#include "BlockLibrary.h"
#include <glm/glm.hpp>
#include <iostream>

using namespace Craft;


void WorldRenderer::GenerateMeshes(
	std::vector<Magma::Vertex>& verticesD,
	std::vector<uint32_t>& indicesD,
	std::vector<Magma::Vertex>& verticesF,
	std::vector<uint32_t>& indicesF,
	Chunk* chunk,
	glm::ivec3 chunkPos)
{
	BlockID* blocks = chunk->blocks;
	uint32_t deferredIndexOffset = verticesD.size();
	uint32_t forwardIndexOffset = verticesF.size();

	for (int i = 0; i < CHUNK_VOLUME; i++)
	{
		// referenced RGL
		
		if (blocks[i] != 0) // if this block isn't air, render
		{
			const Craft::BlockData& curBlockData = Craft::BlockLibrary::GetBlockData(blocks[i]);

			float x = (chunkPos.x * Craft::CHUNK_WIDTH) + chunk->GetBlockX(i);
			float y = (chunkPos.y * Craft::CHUNK_WIDTH) + chunk->GetBlockY(i);
			float z = (chunkPos.z * Craft::CHUNK_WIDTH) + chunk->GetBlockZ(i);
			glm::vec2 uv;

			// index = x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH);
			// mesh class expects pos, normals, texcoords, tangents, and bitangents

			BlockData currentBlock = BlockLibrary::GetBlockData(blocks[i]);

			// EAST FACE | EAST +X
			BlockData eastBlock = Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::EAST)
			);
			if (eastBlock.isTransparent)
			{
				if (!(currentBlock.isTransparent && currentBlock.name == eastBlock.name))
				{
					Magma::Vertex v1, v2, v3, v4;
					uv = BlockLibrary::GetTexCoords(curBlockData.textureEast);

					// top left
					v1.Position = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
					v1.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
					v1.TexCoords = uv;
					v1.Tangent = glm::vec3(0.0f, 0.0f, 1.0f);
					v1.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// top right
					v2.Position = glm::vec3(x + 0.5f, y + 0.5f, z - 0.5f);
					v2.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
					v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
					v2.Tangent = glm::vec3(0.0f, 0.0f, 1.0f);
					v2.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// bottom right
					v3.Position = glm::vec3(x + 0.5f, y - 0.5f, z - 0.5f);
					v3.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
					v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
					v3.Tangent = glm::vec3(0.0f, 0.0f, 1.0f);
					v3.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// bottom left
					v4.Position = glm::vec3(x + 0.5f, y - 0.5f, z + 0.5f);
					v4.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
					v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
					v4.Tangent = glm::vec3(0.0f, 0.0f, 1.0f);
					v4.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// glass/water rule
					// if both my block neighbor and me are glass, dont render me

					if ((currentBlock.isTransparent))
					{
						// if transparent and not next to same transparent block
						verticesF.push_back(v1);
						verticesF.push_back(v2);
						verticesF.push_back(v3);
						verticesF.push_back(v4);

						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset + 1);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 3);

						forwardIndexOffset += 4;
					}
					else
					{
						// if not transparent send to deferred renderer
						verticesD.push_back(v1);
						verticesD.push_back(v2);
						verticesD.push_back(v3);
						verticesD.push_back(v4);

						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset + 1);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 3);

						deferredIndexOffset += 4;
					}
				}
			}



			// WEST FACE | WEST -X
			BlockData westBlock = Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::WEST)
			);
			if (westBlock.isTransparent)
			{
				if (!(currentBlock.isTransparent && currentBlock.name == westBlock.name))
				{
					Magma::Vertex v1, v2, v3, v4;
					uv = BlockLibrary::GetTexCoords(curBlockData.textureWest);

					// top left
					v1.Position = glm::vec3(x - 0.5f, y + 0.5f, z - 0.5f);
					v1.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
					v1.TexCoords = uv;
					v1.Tangent = glm::vec3(0.0f, 0.0f, -1.0f);
					v1.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// top right
					v2.Position = glm::vec3(x - 0.5f, y + 0.5f, z + 0.5f);
					v2.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
					v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
					v2.Tangent = glm::vec3(0.0f, 0.0f, -1.0f);
					v2.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// bottom right
					v3.Position = glm::vec3(x - 0.5f, y - 0.5f, z + 0.5f);
					v3.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
					v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
					v3.Tangent = glm::vec3(0.0f, 0.0f, -1.0f);
					v3.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// bottom left
					v4.Position = glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f);
					v4.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
					v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
					v4.Tangent = glm::vec3(0.0f, 0.0f, -1.0f);
					v4.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// glass/water rule
					// if both my block neighbor and me are glass, dont render me

					if ((currentBlock.isTransparent))
					{
						// if transparent and not next to same transparent block
						verticesF.push_back(v1);
						verticesF.push_back(v2);
						verticesF.push_back(v3);
						verticesF.push_back(v4);

						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset + 1);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 3);

						forwardIndexOffset += 4;
					}
					else
					{
						// if not transparent send to deferred renderer
						verticesD.push_back(v1);
						verticesD.push_back(v2);
						verticesD.push_back(v3);
						verticesD.push_back(v4);

						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset + 1);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 3);

						deferredIndexOffset += 4;
					}
				}
			}

			
			// TOP FACE | UP +Y
			BlockData upBlock = Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::UP)
			);
			if (upBlock.isTransparent)
			{
				if (!(currentBlock.isTransparent && currentBlock.name == upBlock.name))
				{
					Magma::Vertex v1, v2, v3, v4;
					uv = BlockLibrary::GetTexCoords(curBlockData.textureTop);

					// top left
					v1.Position = glm::vec3(x - 0.5f, y + 0.5f, z - 0.5f);
					v1.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
					v1.TexCoords = uv;
					v1.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v1.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);

					// top right
					v2.Position = glm::vec3(x + 0.5f, y + 0.5f, z - 0.5f);
					v2.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
					v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
					v2.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v2.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);

					// bottom right
					v3.Position = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
					v3.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
					v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
					v3.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v3.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);

					// bottom left
					v4.Position = glm::vec3(x - 0.5f, y + 0.5f, z + 0.5f);
					v4.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
					v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
					v4.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v4.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);

					// glass/water rule
					// if both my block neighbor and me are glass, dont render me

					if ((currentBlock.isTransparent))
					{
						// if transparent and not next to same transparent block
						verticesF.push_back(v1);
						verticesF.push_back(v2);
						verticesF.push_back(v3);
						verticesF.push_back(v4);

						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset + 1);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 3);

						forwardIndexOffset += 4;
					}
					else
					{
						// if not transparent send to deferred renderer
						verticesD.push_back(v1);
						verticesD.push_back(v2);
						verticesD.push_back(v3);
						verticesD.push_back(v4);

						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset + 1);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 3);

						deferredIndexOffset += 4;
					}
				}
			}

			// BOTTOM FACE | DOWN -Y
			BlockData downBlock = Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::DOWN)
			);
			if (downBlock.isTransparent)
			{
				if (!(currentBlock.isTransparent && currentBlock.name == downBlock.name))
				{
					Magma::Vertex v1, v2, v3, v4;
					uv = BlockLibrary::GetTexCoords(curBlockData.textureBottom);

					// top left
					v1.Position = glm::vec3(x - 0.5f, y - 0.5f, z + 0.5f);
					v1.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
					v1.TexCoords = uv;
					v1.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v1.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f);

					// top right
					v2.Position = glm::vec3(x + 0.5f, y - 0.5f, z + 0.5f);
					v2.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
					v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
					v2.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v2.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f);

					// bottom right
					v3.Position = glm::vec3(x + 0.5f, y - 0.5f, z - 0.5f);
					v3.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
					v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
					v3.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v3.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f);

					// bottom left
					v4.Position = glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f);
					v4.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
					v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
					v4.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v4.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f);

					// glass/water rule
					// if both my block neighbor and me are glass, dont render me

					if ((currentBlock.isTransparent))
					{
						// if transparent and not next to same transparent block
						verticesF.push_back(v1);
						verticesF.push_back(v2);
						verticesF.push_back(v3);
						verticesF.push_back(v4);

						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset + 1);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 3);

						forwardIndexOffset += 4;
					}
					else
					{
						// if not transparent send to deferred renderer
						verticesD.push_back(v1);
						verticesD.push_back(v2);
						verticesD.push_back(v3);
						verticesD.push_back(v4);

						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset + 1);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 3);

						deferredIndexOffset += 4;
					}
				}
			}

			// SOUTH FACE | SOUTH +Z
			BlockData southBlock = Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::SOUTH)
			);
			if (southBlock.isTransparent)
			{
				if (!(currentBlock.isTransparent && currentBlock.name == southBlock.name))
				{
					Magma::Vertex v1, v2, v3, v4;
					uv = BlockLibrary::GetTexCoords(curBlockData.textureSouth);

					// top left
					v1.Position = glm::vec3(x - 0.5f, y + 0.5f, z + 0.5f);
					v1.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
					v1.TexCoords = uv;
					v1.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v1.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// top right
					v2.Position = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
					v2.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
					v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
					v2.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v2.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// bottom right
					v3.Position = glm::vec3(x + 0.5f, y - 0.5f, z + 0.5f);
					v3.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
					v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
					v3.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v3.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// bottom left
					v4.Position = glm::vec3(x - 0.5f, y - 0.5f, z + 0.5f);
					v4.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
					v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
					v4.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					v4.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// glass/water rule
					// if both my block neighbor and me are glass, dont render me

					if ((currentBlock.isTransparent))
					{
						// if transparent and not next to same transparent block
						verticesF.push_back(v1);
						verticesF.push_back(v2);
						verticesF.push_back(v3);
						verticesF.push_back(v4);

						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset + 1);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 3);

						forwardIndexOffset += 4;
					}
					else
					{
						// if not transparent send to deferred renderer
						verticesD.push_back(v1);
						verticesD.push_back(v2);
						verticesD.push_back(v3);
						verticesD.push_back(v4);

						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset + 1);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 3);

						deferredIndexOffset += 4;
					}
				}
			}

			// NORTH FACE | NORTH -Z
			BlockData northBlock = Craft::BlockLibrary::GetBlockData(
				m_WorldManager.lock()->GetBlockNeighborData(i, chunk, chunkPos, Direction::NORTH)
			);
			if (northBlock.isTransparent)
			{
				if (!(currentBlock.isTransparent && currentBlock.name == northBlock.name))
				{
					Magma::Vertex v1, v2, v3, v4;
					uv = BlockLibrary::GetTexCoords(curBlockData.textureNorth);

					// top left
					v1.Position = glm::vec3(x + 0.5f, y + 0.5f, z - 0.5f);
					v1.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
					v1.TexCoords = uv;
					v1.Tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
					v1.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// top right
					v2.Position = glm::vec3(x - 0.5f, y + 0.5f, z - 0.5f);
					v2.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
					v2.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y);
					v2.Tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
					v2.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// bottom right
					v3.Position = glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f);
					v3.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
					v3.TexCoords = glm::vec2(uv.x + BlockLibrary::m_UVTileScale, uv.y + BlockLibrary::m_UVTileScale);
					v3.Tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
					v3.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// bottom left
					v4.Position = glm::vec3(x + 0.5f, y - 0.5f, z - 0.5f);
					v4.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
					v4.TexCoords = glm::vec2(uv.x, uv.y + BlockLibrary::m_UVTileScale);
					v4.Tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
					v4.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

					// glass/water rule
					// if both my block neighbor and me are glass, dont render me

					if ((currentBlock.isTransparent))
					{
						// if transparent and not next to same transparent block
						verticesF.push_back(v1);
						verticesF.push_back(v2);
						verticesF.push_back(v3);
						verticesF.push_back(v4);

						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset + 1);
						indicesF.push_back(forwardIndexOffset + 2);
						indicesF.push_back(forwardIndexOffset);
						indicesF.push_back(forwardIndexOffset + 3);

						forwardIndexOffset += 4;
					}
					else
					{
						// if not transparent send to deferred renderer
						verticesD.push_back(v1);
						verticesD.push_back(v2);
						verticesD.push_back(v3);
						verticesD.push_back(v4);

						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset + 1);
						indicesD.push_back(deferredIndexOffset + 2);
						indicesD.push_back(deferredIndexOffset);
						indicesD.push_back(deferredIndexOffset + 3);

						deferredIndexOffset += 4;
					}
				}
			}
		}
	}
}

bool WorldRenderer::AddMeshToDeferredDrawPool(std::unique_ptr<Magma::Mesh> mesh, glm::ivec3 chunkPos)
{
	// if already in draw pool
	if (m_DeferredDrawPool.contains(chunkPos)) return false;

	m_DeferredDrawPool[chunkPos] = std::move(mesh);
	return true;
}

void WorldRenderer::RemoveFromDeferredDrawPool(glm::ivec3 chunkPos)
{
	m_DeferredDrawPool.erase(chunkPos);
}

bool WorldRenderer::AddMeshToForwardDrawPool(std::unique_ptr<Magma::Mesh> mesh, glm::ivec3 chunkPos)
{
	// if already in draw pool
	if (m_ForwardDrawPool.contains(chunkPos)) return false;

	m_ForwardDrawPool[chunkPos] = std::move(mesh);
	return true;
}

void WorldRenderer::RemoveFromForwardDrawPool(glm::ivec3 chunkPos)
{
	m_ForwardDrawPool.erase(chunkPos);
}

bool WorldRenderer::AddMeshesToDrawPool(std::unique_ptr<Magma::Mesh> dmesh, std::unique_ptr<Magma::Mesh> fmesh, glm::ivec3 chunkPos)
{
	bool deferredSuccess = AddMeshToDeferredDrawPool(std::move(dmesh), chunkPos);
	bool forwardSuccess = AddMeshToForwardDrawPool(std::move(fmesh), chunkPos);

	return deferredSuccess && forwardSuccess;
}

void WorldRenderer::RemoveFromDrawPools(glm::ivec3 chunkPos)
{
	RemoveFromDeferredDrawPool(chunkPos);
	RemoveFromForwardDrawPool(chunkPos);
}

void WorldRenderer::DrawDeferredWorld()
{
	if (m_DeferredDrawPool.empty()) return;

	// Calling Mesh->Draw()
	for (const auto& pair : m_DeferredDrawPool)
	{
		pair.second->Draw();
	}
}

void WorldRenderer::DrawForwardWorld(glm::vec3 camPos)
{
	if (m_ForwardDrawPool.empty()) return;

	std::vector<std::pair<glm::vec3, Magma::Mesh*>> transparentMeshes;
	transparentMeshes.reserve(m_ForwardDrawPool.size());

	// fill mesh pointer vector
	for (const auto& pair : m_ForwardDrawPool)
	{
		transparentMeshes.push_back({ glm::vec3(pair.first), pair.second.get() });
	}

	// sort from furthest to closest to camera
	std::sort(transparentMeshes.begin(), transparentMeshes.end(),
		[&camPos](const std::pair<glm::vec3, Magma::Mesh*>& a, const std::pair<glm::vec3, Magma::Mesh*>& b)
		{
			glm::vec3 diffA = camPos - a.first;
			glm::vec3 diffB = camPos - b.first;

			float distSqA = glm::dot(diffA, diffA);
			float distSqB = glm::dot(diffB, diffB);

			return distSqA > distSqB;
		});

	// Calling Mesh->Draw()
	for (const auto& pair : transparentMeshes)
	{
		pair.second->Draw();
	}
}