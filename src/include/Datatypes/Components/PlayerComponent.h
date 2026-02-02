#pragma once
#include <string>
#include <cstdint>

namespace Magma { class Texture; }

namespace Craft
{
	struct PlayerComponent
	{
		uint8_t networkID; // we lookup username through network manager
		bool isLocalPlayer = false;
		uint32_t lastSequenceID = 0; // for packets
		Magma::Texture* texture = nullptr;
	};
}