#pragma once

#include <Datatypes/ScriptableEntity.h>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

namespace Craft
{
	class PlayerController : public ScriptableEntity
	{
		public:
			void OnAttach() override;
			void OnUpdate(float dt) override;
			void SetWindow(SDL_Window* window) { m_Window = window; }

		private:
			void HandleMovement(float dt);
			void HandleMouseLook(float dt);

			float m_MoveSpeed = 5.0f;
			float m_MouseSensitivity = 0.1f;

			EntityID m_CameraEntity = NULL_ENTITY;
			float m_Yaw = 0.0f;
			float m_Pitch = 0.0f;
			SDL_Window* m_Window;

			glm::vec3 m_LastPosition = glm::vec3(0.0f);
	};
}