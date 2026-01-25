#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

// Uses SDL to track input
// currently only implemented for Keyboard and Mouse
namespace Magma
{
	class Input
	{
		public:
			static void Init();
			static void Update();

			// Keyboard Input
			static bool IsKeyHeld(SDL_Scancode key);
			static bool IsKeyPressed(SDL_Scancode key);
			static bool IsKeyReleased(SDL_Scancode key);

			// Mouse Input
			static bool IsMouseButtonHeld(Uint8 button);
			static bool IsMouseButtonPressed(Uint8 button);
			static bool IsMouseButtonReleased(Uint8 button);

			static glm::vec2 GetMousePosition();
			static glm::vec2 GetMouseDelta();

		private:
			// Keyboard state
			static const bool* m_CurrentKeyState;
			static bool m_PreviousKeyState[SDL_SCANCODE_COUNT];

			// Mouse State
			static float m_MouseX, m_MouseY;
			static float m_MouseDeltaX, m_MouseDeltaY;
			static Uint32 m_CurrentMouseState;
			static Uint32 m_PreviousMouseState;
	};
}