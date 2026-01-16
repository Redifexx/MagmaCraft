#pragma once
#include <enet/enet.h>

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
#include <Client.h>
#include <Server.h>
#include "WorldManager.h"

namespace Magma
{

	enum class MenuState
	{
		MAIN_MENU,
		SINGLEPLAYER,
		MULTIPLAYER,
		CREATE_WORLD,
		HOST_GAME,
		JOIN_GAME,
		LOADING,
		IN_GAME
	};

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
			MenuState m_MenuState = MenuState::MAIN_MENU;
			char m_SeedBuf[32] = "";
			bool m_AutoSeed = true;
			Craft::WorldManager* m_WorldManager = nullptr;
			char m_WorldNameBuf[32] = "";
			Craft::NetworkManger* m_NetworkManager = nullptr;


			// Demo Variables
			// These should ideally be part of another class or system
			glm::mat4 m_ModelMatrix;
			Camera* m_Camera;
			Texture* m_Texture;
			SDL_Window* m_Window;

			// maybe turn into a network manager class later
			Client* m_Client;
			Server* m_Server;
			bool m_Host; //server true, client false
			bool m_NetworkInitialized = false;

			// ConnectionStuff
			ConnectionState m_ConnectionState = ConnectionState::DISCONNECTED;
			float m_ConnectionTimer = 0.0f;
			const float CONNECTION_TIMEOUT = 5.0f;
			bool m_AutoConnect = true;

			char m_NetworkMsg[256] = "";
			char m_ServerAddressBuf[256] = "";
			char m_ServerportBuf[256] = "";
			char m_ChatLog[1024 * 16] = ""; // big buffer for chat log
	};
}