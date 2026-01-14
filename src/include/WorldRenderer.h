#pragma once

namespace Craft
{
	class WorldRenderer
	{
		public:

		private:
			// will determine chunk buffer size
			// will be cached from server
			uint8_t m_ChunkRenderDistance = 8;
	};
}