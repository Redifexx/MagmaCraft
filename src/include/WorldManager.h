#pragma once

// Manages world data, including loading, saving, and updating chunks
namespace Craft
{
	class WorldManager
	{
		public:
			WorldManager();
			void CreateWorld(const std::string& worldName, int seed);
			void LoadWorld(const std::string& worldName);
			
		private:
			
	};
}