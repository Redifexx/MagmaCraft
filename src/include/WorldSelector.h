#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <imgui.h>


// class for world browser ui
namespace Craft
{
	struct WorldEntry
	{
		std::string name;
		std::string path;
		std::string displayString;
		std::string thumbnail;
	};

	class WorldSelector
	{
		public:
			void Refresh();
			bool OnImGuiRender(std::string& worldPath);
		private:
			std::vector<WorldEntry> m_Worlds;
	};
}