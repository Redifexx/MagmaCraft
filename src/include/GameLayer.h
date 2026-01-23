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
#include "WorldStreamer.h"
#include <memory>
#include <random>

namespace Magma
{
	// header only random helper class, may need a new home
	class Random
	{
		public:
			Random() = delete; // no constructor

			static int Int(int min, int max)
			{
				std::uniform_int_distribution<int> dist(min, max);
				return dist(GetEngine());
			}

			// Returns float between 0.0, 1.0
			static float Float()
			{
				std::uniform_real_distribution<float> dist(0.0f, 1.0f);
				return dist(GetEngine());
			}

		private:
			static std::mt19937& GetEngine()
			{
				static std::random_device rd;
				static std::mt19937 engine(rd());
				return engine;
			}
	};

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
			std::unique_ptr<ShaderProgram> m_ShaderProgram = nullptr;
			MenuState m_MenuState = MenuState::MAIN_MENU;

			std::shared_ptr<Craft::NetworkManager> m_NetworkManager = nullptr;
			std::unique_ptr<Craft::WorldStreamer> m_WorldStreamer = nullptr;

			// Text Input Buffers
			char m_SeedBuf[32] = "";
			char m_WorldNameBuf[32] = "";
			char m_NetworkMsg[256] = "";
			char m_ServerAddressBuf[256] = "";
			char m_ServerportBuf[256] = "";
			char m_ChatLog[1024 * 16] = ""; // big buffer for chat log

			// IMGui Options
			bool m_AutoConnect = true;
			bool m_AutoSeed = true;

			// Demo Variables
			// These should ideally be part of another class or system
			glm::mat4 m_ModelMatrix;
			std::unique_ptr<Camera> m_Camera = nullptr;
			std::unique_ptr<Texture> m_Texture = nullptr;
			SDL_Window* m_Window; // make shared
	};
}