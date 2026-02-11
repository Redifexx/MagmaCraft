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
#include "imfilebrowser.h"
#include <Datatypes/EntityWorld.h>
#include <Systems/TransformSystem.h>
#include <Systems/CameraSystem.h>
#include <Systems/RenderSystem.h>
#include <Systems/ScriptSystem.h>
#include <Datatypes/Components/CameraComponent.h>
#include <glui/glui.h>
#include "gl2d/gl2d.h"
#include "WorldSelector.h"

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

			static unsigned int UInt(unsigned int min, unsigned int max)
			{
				std::uniform_int_distribution<unsigned int> dist(min, max);
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
		SET_NAME,
		MAIN_MENU,
		SINGLEPLAYER,
		MULTIPLAYER,
		CREATE_WORLD,
		LOAD_WORLD,
		HOST_GAME,
		JOIN_GAME,
		LOADING,
		IN_GAME
	};

	// This is where the main loop game logic should go
	class GameLayer : public Layer
	{
		public:
			GameLayer(SDL_Window* window) { SetWindow(window); }
			void OnAttach() override;
			void OnUpdate(float dt) override;
			void OnDetach() override;
			void OnImGuiRender(float dt) override;
			void OnResize(int width, int height);

		private:
			void SetWindow(SDL_Window* window) { m_Window = window; }
			void SpawnLocalPlayer();
			void CleanupLocalPlayer();
			void WorldShutdown();
			void RenderUI(const int& w, const int& h, float dt);
			void SetupShadowMap();
			void Screenshot(const int& w, const int& h);
			void ToggleVSync();

			bool m_VSyncEnabled = true;

			std::vector<Model*> m_Models;
			std::unique_ptr<ShaderProgram> m_ShaderProgram = nullptr;
			MenuState m_MenuState = MenuState::SET_NAME;

			std::shared_ptr<Craft::NetworkManager> m_NetworkManager = nullptr;
			std::unique_ptr<Craft::WorldStreamer> m_WorldStreamer = nullptr;

			// Text Input Buffers
			std::unique_ptr<Craft::WorldSelector> m_WorldSelector = nullptr;
			char m_SeedBuf[32] = "";
			char m_WorldNameBuf[32] = "";
			char m_UserNameBuf[32] = "Redifexx";
			char m_WorldPathBuf[128] = "";
			char m_NetworkMsg[256] = "";
			char m_ServerAddressBuf[256] = "";
			char m_ServerportBuf[256] = "";
			char m_ChatLog[1024 * 16] = ""; // big buffer for chat log

			// Timers
			float m_AutoSaveTimer = 0.0f;
			float m_AutoSaveRate = 180.0f;
			float m_NetworkTickTimer = 0.0f;
			float m_NetworkTickRate = 1.0f / 20.0f;
			float m_ConnectionFailTimer = 0.0f;
			float m_ConnectionFailRate = 3.0f;

			// Framerate stuff
			float m_TotalFPS = 0.0f;
			int m_FrameCount = 0;
			float m_TotalFrameTime = 0.0f;
			float m_AvgFPS = 0.0f;
			float m_AvgFrameTime = 0.0f;

			// IMGui Options
			bool m_AutoConnect = true;
			bool m_AutoSeed = true;
			ImGui::FileBrowser m_FileBrowser;

			// Demo Variables
			// These should ideally be part of another class or system
			glm::mat4 m_ModelMatrix;
			std::unique_ptr<Camera> m_Camera = nullptr;
			Model* pModel = nullptr;
			SDL_Window* m_Window = nullptr; // make shared

			// Entity World
			std::shared_ptr<Craft::EntityWorld> m_EntityWorld = nullptr;
			std::unique_ptr<Craft::TransformSystem> m_TransformSystem = nullptr;
			std::unique_ptr<Craft::CameraSystem> m_CameraSystem = nullptr;
			std::unique_ptr<Craft::RenderSystem> m_RenderSystem = nullptr;
			std::unique_ptr<Craft::ScriptSystem> m_ScriptSystem = nullptr;

			uint32_t m_Player = Craft::NULL_ENTITY;
			uint32_t m_PrimaryCamera = Craft::NULL_ENTITY;
			std::string m_Username = "Player";
			bool m_IsLocalPlayerLoaded = false;
		
			// here for now because i dont have a resource manager YET
			unsigned int m_ScreenFBO;
			unsigned int m_ScreenRBO;
			std::unique_ptr<Magma::Texture> m_ScreenTextureColorBuffer = nullptr;
			unsigned int m_ScreenVAO, m_ScreenVBO;
			// my mesh class doesnt support a lack of normals :(
			void CreateScreenQuad(unsigned int& vao, unsigned int& vbo);
			std::unique_ptr<Magma::Mesh> m_ScreenQuad = nullptr;
			std::unique_ptr<ShaderProgram> m_ScreenShaderProgram = nullptr;

			// Deferred rendering stuff (learn open gl)
			unsigned int m_GBuffer;
			std::unique_ptr<Magma::Texture> m_GPosition = nullptr;
			std::unique_ptr<Magma::Texture> m_GNormal = nullptr;
			std::unique_ptr<Magma::Texture> m_GAlbedo = nullptr;
			std::unique_ptr<Magma::Texture> m_GMatData = nullptr;

			unsigned int m_GLightingPassFBO;
			std::unique_ptr<Magma::Texture> m_GLightingPass = nullptr;
			std::unique_ptr<Magma::Texture> m_GDepth = nullptr;
			std::unique_ptr<ShaderProgram> m_LightingShaderProgram = nullptr;

			// DEBUG MENU ITEMS
			float m_FogNear = 0.1f;
			float m_FogFar = 1000.0f;
			float m_FogDensity = 0.007f;
			float m_FogCurve = 2.0f;

			glm::vec3 m_SkyColor = glm::vec3(0.3, 0.5, 1.0);
			float m_SunIntensity = 2.0f;
			glm::vec3 m_SunColor = glm::vec3(1.0f);
			glm::vec3 m_SunDirection = glm::vec3(-0.5f);

			// Shadow Map stuff
			unsigned int m_ShadowMapFBO = 0;
			std::unique_ptr<Magma::Texture> m_ShadowMap = nullptr;
			const int SHADOW_MAP_RESOLUTION = 2048;
			std::unique_ptr<ShaderProgram> m_ShadowMapShaderProgram = nullptr;
			float m_SunShadowNearPlane = 3.0f;
			float m_SunShadowFarPlane = 400.0f;
			float m_SunShadowOrthoSize = 50.0f;
			float m_SunDistanceMultiplier = 350.0f;
			float m_ShadowBiasMin = 0.0001f;
			float m_ShadowBiasMax = 0.0001f;
			float m_ShadowFadeDistance = 50.0f;
			glm::mat4 m_LightProjMatrix = glm::mat4(1.0f);
			glm::mat4 m_LightViewMatrix = glm::mat4(1.0f);
			bool m_LightProjDirty = true;


			// Should later be moved to manager
			glui::RendererUi* m_UI = nullptr;
			gl2d::Renderer2D* m_UIRenderer = nullptr;
			gl2d::Font* m_UIFont = nullptr;
			gl2d::Texture* m_UITexture = nullptr;
	};
}