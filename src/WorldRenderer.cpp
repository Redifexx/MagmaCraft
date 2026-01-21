#include "WorldRenderer.h"
#include "BlockLibrary.h"

using namespace Craft;

WorldRenderer::WorldRenderer()
{
}

void WorldRenderer::RenderChunk(Chunk* chunk)
{
	std::unique_ptr<Magma::Mesh> mesh = std::make_unique<Magma::Mesh>();
	GenerateMesh(*mesh, chunk->blocks);

}

void WorldRenderer::GenerateMesh(Magma::Mesh& mesh, BlockID* blocks)
{
	for (int i = 0; i < CHUNK_VOLUME; i++)
	{
		// reference rgl
		
		if (blocks[i] != 0) // if this block isn't air, render
		{
			const Craft::BlockData& curBlockData = Craft::BlockLibrary::GetBlockData(blocks[i]);

		}
	}
}
