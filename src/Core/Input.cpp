#include "Core/Input.h"
#include <iostream>

namespace Magma
{
	const bool* Input::m_CurrentKeyState = nullptr;
	bool Input::m_PreviousKeyState[SDL_SCANCODE_COUNT] = { false };

	float Input::m_MouseX = 0.0f;
	float Input::m_MouseY = 0.0f;
	float Input::m_MouseDeltaX = 0.0f;
	float Input::m_MouseDeltaY = 0.0f;
	Uint32 Input::m_CurrentMouseState = 0;
	Uint32 Input::m_PreviousMouseState = 0;

	void Input::Init()
	{
		// Grab keyboard state pointer from SDL, it's updated by SDL internally
		int numKeys;
		m_CurrentKeyState = SDL_GetKeyboardState(&numKeys);

		memset(m_PreviousKeyState, 0, sizeof(m_PreviousKeyState));
	}

	void Input::Update()
	{
		// Copy current key state to previous key state
		memcpy(m_PreviousKeyState, m_CurrentKeyState, sizeof(m_PreviousKeyState));
		m_PreviousMouseState = m_CurrentMouseState;

		float x, y;
		SDL_GetRelativeMouseState(&x, &y);
		m_MouseDeltaX = x;
		m_MouseDeltaY = y;

		// Update mouse position
		m_CurrentMouseState = SDL_GetMouseState(&m_MouseX, &m_MouseY);
	}

	// Keyboard
	bool Input::IsKeyHeld(SDL_Scancode key)
	{
		// Pass nullptr to get the internal array of the keyboard state
		//const Uint8* state = SDL_GetKeyboardState(nullptr);

		// Check if the specific key is physically down right now
		//return state[scancode] != 0;
		//std::cout << "holding: " << key << " state: " << m_CurrentKeyState[key] << std::endl;
		return m_CurrentKeyState[key];
	}

	bool Input::IsKeyPressed(SDL_Scancode key)
	{
		return m_CurrentKeyState[key] && !m_PreviousKeyState[key];
	}

	bool Input::IsKeyReleased(SDL_Scancode key)
	{
		return !m_CurrentKeyState[key] && m_PreviousKeyState[key];
	}

	// sdl_button_left/right
	// Mouse
	bool Input::IsMouseButtonHeld(Uint8 button)
	{
		return (m_CurrentMouseState & SDL_BUTTON_MASK(button));
	}

	bool Input::IsMouseButtonPressed(Uint8 button)
	{
		bool isDown = (m_CurrentMouseState & SDL_BUTTON_MASK(button));
		bool wasDown = (m_PreviousMouseState & SDL_BUTTON_MASK(button));
		return isDown && !wasDown;
	}

	bool Input::IsMouseButtonReleased(Uint8 button)
	{
		bool isDown = (m_CurrentMouseState & SDL_BUTTON_MASK(button));
		bool wasDown = (m_PreviousMouseState & SDL_BUTTON_MASK(button));
		return !isDown && wasDown;
	}

	glm::vec2 Input::GetMousePosition()
	{
		return glm::vec2(m_MouseX, m_MouseY);
	}

	glm::vec2 Input::GetMouseDelta()
	{
		return glm::vec2(m_MouseDeltaX, m_MouseDeltaY);
	}
}