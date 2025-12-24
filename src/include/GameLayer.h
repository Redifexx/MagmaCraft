#pragma once

#include "Server.h"
#include "Client.h"
#include "Layer.h"
#include <SDL3/SDL.h>
#include "imgui.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <vector>
#include "Model.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include "Input.h"
#include "Window.h"

namespace Magma
{
	// This is where the main loop game logic should go
	class GameLayer : public Layer
	{
		public:
			void OnAttach() override;
			void OnUpdate(float dt) override;
			void OnDetach() override;
			void OnImGuiRender() override;
			void OnResize(int width, int height);
			void SetWindow(SDL_Window* window) { m_Window = window; }

		private:
			std::vector<Model*> m_Models;
			ShaderProgram* m_ShaderProgram;

			// Demo Variables
			// These should ideally be part of another class or system
			glm::mat4 m_ModelMatrix;
			Camera* m_Camera;
			Texture* m_Texture;
			SDL_Window* m_Window;
			Server* m_Server;
			Client* m_Client;
			bool m_Host; //server true, client false
			std::string m_ClientMsg;
	};
}