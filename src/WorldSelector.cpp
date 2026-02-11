#include "WorldSelector.h"

using namespace Craft;

void WorldSelector::Refresh()
{
	m_Worlds.clear();
	std::string saveFolder = "saves/";

	if (!std::filesystem::exists(saveFolder))
	{
		std::filesystem::create_directory(saveFolder);
	}

	for (const auto& entry : std::filesystem::directory_iterator(saveFolder))
	{
		if (entry.is_directory())
		{
			WorldEntry world;
			world.name = entry.path().filename().string();
			world.path = entry.path().string() + "/" + world.name + ".mcwd";

			world.displayString = world.name;
			world.thumbnail = "";

			m_Worlds.push_back(world);
		}
	}
}

bool WorldSelector::OnImGuiRender(std::string& worldPath)
{
	bool isSelected = false;

	ImGui::Text("Select World");
	if (ImGui::Button("Refresh List") || m_Worlds.empty())
	{
		Refresh();
	}

	ImGui::Separator();

	for (const auto& world : m_Worlds)
	{
		if (ImGui::Button(world.displayString.c_str(), ImVec2(-1, 40)))
		{
			worldPath = world.path;
			isSelected = true;
		}
	}


	return isSelected;
}