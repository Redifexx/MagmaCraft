#pragma once
#include <string>

namespace Craft
{
	struct PlayerComponent
	{
		uint32_t playerID;
		std::string username;
		bool isLocalPlayer = false;
	};
}