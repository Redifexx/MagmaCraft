#pragma once
#include <string>

namespace Craft
{
	struct PlayerComponent
	{
		uint8_t networkID; // we lookup username through network manager
		bool isLocalPlayer = false;
		uint32_t lastSequenceID = 0; // for packets
	};
}