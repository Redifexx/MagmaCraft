#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Craft
{
	struct TransformComponent
	{
		glm::vec3 localPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat localRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 localScale = glm::vec3(1.0f, 1.0f, 1.0f);

		glm::mat4 localMatrix = glm::mat4(1.0f);
		glm::mat4 worldMatrix = glm::mat4(1.0f);
	
		bool isDirty = true;

		void SetPosition(glm::vec3 pos) { localPosition = pos; isDirty = true; }
		void SetRotation(glm::quat rot) { localRotation = rot; isDirty = true; }
		void SetScale(glm::vec3 scale) { localScale = scale; isDirty = true; }
	};
}