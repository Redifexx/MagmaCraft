#pragma once

#include <string>
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include "imgui.h"
#include <backends/imgui_impl_sdl3.h>
#include <iostream>
#include <functional>

namespace Magma
{
	class Window
	{
		public:
			bool Init(int width, int height, const std::string& title);
			void SwapBuffers();
			void PollEvents(bool& isRunning);
			void Shutdown();

			void SetVSync(int vSync);

			SDL_Window* GetSDLWindow() const { return m_Window; }
			SDL_GLContext GetGLContext() const { return m_GLContext; }
			void GetWindowSize(int& width, int& height) const;

			using ResizeCallbackFn = std::function<void(int, int)>;
			void SetResizeCallback(const ResizeCallbackFn& callback) { m_ResizeCallback = callback; }

		private:
			SDL_Window* m_Window = nullptr;
			SDL_GLContext m_GLContext = nullptr;
			ResizeCallbackFn m_ResizeCallback;
	};
}