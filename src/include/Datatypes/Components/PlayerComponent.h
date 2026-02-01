#pragma once
#include <string>

namespace Craft
{
	struct PlayerComponent
	{
		std::string username;
		bool isLocalPlayer = false;
		uint32_t lastSequenceID = 0; // for packets
	};
}