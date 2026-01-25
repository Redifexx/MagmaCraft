#pragma once

#include "Layer.h"
#include <SDL3/SDL.h>
#include "imgui.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>

namespace Magma
{
	class ImGuiLayer : public Layer
	{
		public:
			ImGuiLayer(SDL_Window* window, SDL_GLContext glContext)
				: Layer("ImGuiLayer"), m_Window(window), m_GLContext(glContext)
			{}

			void OnAttach() override;
			void OnDetach() override;
			void Begin(); // on frame start
			void End(); // on frame end
		private:
			SDL_Window* m_Window = nullptr;
			SDL_GLContext m_GLContext = nullptr;
	};
}