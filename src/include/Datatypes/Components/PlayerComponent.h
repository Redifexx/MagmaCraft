#pragma once
#include <string>
#include <cstdint>

namespace Magma { class Texture; } // forward declaration

namespace Craft
{
	struct PlayerComponent
	{
		uint8_t networkID; // we lookup username through network manager
		bool isLocalPlayer = false;
		uint32_t lastSequenceID = 0; // for packets
		Magma::Texture* texture = nullptr;

		// some variables needed for movement interpolation
		glm::vec3 startPos = glm::vec3(0.0f);
		glm::vec3 targetPos = glm::vec3(0.0f);

		glm::quat startRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::quat targetRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

		float interpolationTime = 0.0f;
		float interpolationDuration = 0.075f; // 0.075 feels right for now
	};
}