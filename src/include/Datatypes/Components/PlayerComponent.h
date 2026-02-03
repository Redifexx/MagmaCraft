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

		// interpolation stats
		glm::vec3 startPos = glm::vec3(0.0f);
		glm::vec3 targetPos = glm::vec3(0.0f);

		glm::quat startRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::quat targetRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

		float interpolationTime = 0.0f;
		float interpolationDuration = 0.075f; // experiment with this
	};
}