#include <enet/enet.h>
#include "Core/GameLayer.h"

#include "Core/Model.h"
#include "Core/ShaderProgram.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include <filesystem>
#include <Core/AudioEngine.h>
#include "WorldManager.h"
#include "NetworkManager.h"
#include "BlockLibrary.h"
#include <limits.h>
#include <cstdio>
#include <Datatypes/EntityWorld.h>
#include <Datatypes/Components/TransformComponent.h>
#include <Datatypes/Components/RelationshipComponent.h>
#include <Datatypes/Components/ModelComponent.h>
#include <Datatypes/Components/CameraComponent.h>
#include <Datatypes/Components/HealthComponent.h>
#include <Scripts/PlayerController.h>
#include <Datatypes/Components/NativeScriptComponent.h>
#include <Datatypes/Components/PlayerComponent.h>

using namespace Magma;

void GameLayer::OnAttach()
{
	// ---- NETWORK INITIALIZATION ----

	if (enet_initialize() != 0)
	{
		std::cout << "Error initializing enet...";
		return;
	}

	// ImGui File Brower Config
	m_FileBrowser.SetTitle("World Browser");
	m_FileBrowser.SetTypeFilters({ ".mcwd" });


	// ---- GAME INITIALIZATION ----

	// ECS SETUP
	// create our entity world (world simulation / logic, NOT the world generation)
	
	// setup systems
	m_EntityWorld = std::make_shared<Craft::EntityWorld>();
	m_TransformSystem = std::make_unique<Craft::TransformSystem>();
	m_RenderSystem = std::make_unique<Craft::RenderSystem>();
	m_CameraSystem = std::make_unique<Craft::CameraSystem>();
	m_ScriptSystem = std::make_unique<Craft::ScriptSystem>();


	// add player entity
	uint32_t playerEntity = m_EntityWorld->AddEntity();

	// add components to player entity
	m_EntityWorld->AddComponent<Craft::TransformComponent>(playerEntity,
		{
			glm::vec3(0.0f, 64.0f, 0.0f), // position
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f), // rotation quat
			glm::vec3(1.0f, 1.0f, 1.0f) // scale
		});

	m_EntityWorld->AddComponent<Craft::HealthComponent>(playerEntity, { 100.0f, 100.0f });

	Magma::Model* playerModelTemp = new Magma::Model("resources/models/player.fbx");
	//m_EntityWorld->AddComponent<Craft::ModelComponent>(playerEntity, { std::move(playerModelTemp) });
	m_Player = playerEntity;


	m_EntityWorld->AddComponent<Craft::PlayerComponent>(playerEntity,
		{
			static_cast<uint32_t>(Magma::Random::UInt(0, UINT_MAX)), // player is given random ID (later add function to avoid duplicates)
			"LocalPlayer",
			true
		});

	// add camera entity

	uint32_t cameraEntity = m_EntityWorld->AddEntity();
	m_EntityWorld->AddComponent<Craft::TransformComponent>(cameraEntity,
		{
			glm::vec3(0.0f, 1.0f, 0.0f), // position
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f), // rotation quat
			glm::vec3(1.0f, 1.0f, 1.0f) // scale
		});

	m_EntityWorld->AddComponent<Craft::CameraComponent>(cameraEntity, {});
	m_EntityWorld->GetComponent<Craft::CameraComponent>(cameraEntity).isPrimary = true; // set primary camera
	m_PrimaryCamera = cameraEntity;

	// make camera a child of player
	m_EntityWorld->AddComponent<Craft::RelationshipComponent>(playerEntity,
		{
			Craft::NULL_ENTITY,
			cameraEntity,
			Craft::NULL_ENTITY,
			Craft::NULL_ENTITY
		});

	m_EntityWorld->AddComponent<Craft::RelationshipComponent>(cameraEntity,
		{
			playerEntity,
			Craft::NULL_ENTITY,
			Craft::NULL_ENTITY,
			Craft::NULL_ENTITY
		});

	// bind scripts
	auto& scriptComponent = m_EntityWorld->AddComponent<Craft::NativeScriptComponent>(playerEntity, {});
	scriptComponent.Bind<Craft::PlayerController>();

	// set script window
	SDL_Window* windowPtr = m_Window;
	scriptComponent.instantiateScript = [windowPtr]()
	{
		auto* controller = new Craft::PlayerController();
		controller->SetWindow(windowPtr);
		return static_cast<Craft::ScriptableEntity*>(controller);
	};


	// Shader setup (Shader.h & ShaderProgram.h)
	std::string vertpath = "resources/shaders/basic.vert";
	std::string fragpath = "resources/shaders/block.frag";
	std::string texturePath = "resources/textures/terrain.png";
	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
		texturePath = std::string(MAGMA_ROOT_DIR) + texturePath;
	#endif

	Shader vertexShader(vertpath, GL_VERTEX_SHADER);
	Shader fragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_ShaderProgram = std::make_unique<ShaderProgram>();

	m_ShaderProgram->AttachShader(vertexShader);
	m_ShaderProgram->AttachShader(fragmentShader);
	if (!m_ShaderProgram->Link())
	{
		std::cerr << "Failed to link shader program!" << std::endl;
		return;
	}

	// Network Manager Setup
	m_NetworkManager = std::make_shared<Craft::NetworkManager>();
	m_WorldStreamer = std::make_unique<Craft::WorldStreamer>(m_NetworkManager);

	//BlockLibrary Setup
	Craft::BlockLibrary::Initialize();

	// Single Texture setup
	m_Texture = std::make_unique<Texture>(texturePath.c_str(), false);
	m_Texture->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_Texture->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Texture->GetID());
	m_ShaderProgram->SetUniform("u_Texture", 0);
}

// ---- GAME UPDATE LOGIC ----
void GameLayer::OnUpdate(float dt)
{
	if (dt > 0.1f) dt = 0.1f; // safaty

	m_ScriptSystem->Update(*m_EntityWorld, dt);
	m_CameraSystem->Update(*m_EntityWorld);

	auto& camTransform = m_EntityWorld->GetComponent<Craft::TransformComponent>(m_PrimaryCamera);
	// ---- INPUT ----
	// Basic input handling for Camera movement (Input.h)
	// Should probably be handled by a manager class

	if (Magma::Input::IsKeyPressed(SDL_SCANCODE_T))
	{
		//Magma::AudioEngine::PlayGlobal("resources/audio/music_6.ogg", 0.1f, true);
		//Magma::AudioEngine::PlayAtLocation("resources/audio/pickitup.mp3", glm::vec3(0.0f, 0.0f, 0.0f), 1.5f, true);
	}

	if (Magma::Input::IsKeyPressed(SDL_SCANCODE_G))
		Magma::AudioEngine::StopGlobal();

	// Mouse look
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse && Magma::Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
	{
		SDL_SetWindowRelativeMouseMode(m_Window, true);
	}

	// ---- WORLD UPDATE ---- FINISH
	m_WorldStreamer->Update(dt, glm::vec3(camTransform.worldMatrix[3]));

	// ---- AUDIO UPDATE ----
	// Audio Listener Update
	Magma::AudioEngine::UpdateListener(glm::vec3(camTransform.worldMatrix[3]), -glm::vec3(camTransform.worldMatrix[2]), glm::vec3(camTransform.worldMatrix[1]));

	// --- TRANSFORMS UPDATE ----
	m_TransformSystem->Update(*m_EntityWorld);

	// ---- RENDERING ----
	// Shader uniforms update and model drawing
	m_ShaderProgram->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Texture->GetID());
	m_ShaderProgram->SetUniform("u_Texture", 0);

	m_RenderSystem->Render(*m_EntityWorld, *m_ShaderProgram, m_WorldStreamer.get(), m_Window);

	Magma::Input::Update();
	Magma::AudioEngine::UpdateActiveSounds();

	// ---- NETWORK UPDATE ----
	if (m_NetworkManager->IsRunning())
	{
		m_NetworkManager->Update(dt);
	}
}

void GameLayer::OnDetach()
{
	// --- GAME CLEANUP LOGIC ----

	for (Model* model : m_Models)
	{
		delete model;
	}
	m_Models.clear();
}

static const char* logoArt = R"(
___  ___  ___  _____ ___  ___  ___  _____ ______  ___  ______ _____ 
|  \/  | / _ \|  __ \|  \/  | / _ \/  __ \| ___ \/ _ \ |  ___|_   _|
| .  . |/ /_\ \ |  \/| .  . |/ /_\ \ /  \/| |_/ / /_\ \| |_    | |  
| |\/| ||  _  | | __ | |\/| ||  _  | |    |    /|  _  ||  _|   | |  
| |  | || | | | |_\ \| |  | || | | | \__/\| |\ \| | | || |     | |  
\_|  |_/\_| |_/\____/\_|  |_/\_| |_/\____/\_| \_\_| |_/\_|     \_/  
)";

void GameLayer::OnImGuiRender()
{
	// --- IMGUI RENDERING ----
	static char buf[256] = "";
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("MagmaCraft");
	if (m_MenuState != MenuState::IN_GAME)
	{
		ImGui::TextUnformatted(logoArt);
		ImGui::Text("by Gio Perez Colon");
	}
	else
	{
		ImGui::Text("MagmaCraft by Gio Perez Colon");
	}

	switch (m_MenuState)
	{
		case (MenuState::MAIN_MENU):
			if (ImGui::Button("Singleplayer"))
			{
				m_MenuState = MenuState::SINGLEPLAYER;
			}
			if (ImGui::Button("Multiplayer"))
			{
				m_MenuState = MenuState::MULTIPLAYER;
			}
			break;

		case (MenuState::SINGLEPLAYER):
			if (ImGui::Button("Create World"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::SERVER);
				m_MenuState = MenuState::CREATE_WORLD;
			}
			if (ImGui::Button("Load World"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::SERVER);
				m_MenuState = MenuState::LOAD_WORLD;
			}
			if (ImGui::Button("Back"))
			{
				m_MenuState = MenuState::MAIN_MENU;
			}
			break;

		case (MenuState::MULTIPLAYER):
			if (ImGui::Button("Host Game"))
			{
				m_MenuState = MenuState::HOST_GAME;
			}
			if (ImGui::Button("Join Game"))
			{
				m_MenuState = MenuState::JOIN_GAME;
			}
			if (ImGui::Button("Back"))
			{
				m_MenuState = MenuState::MAIN_MENU;
			}
			break;

		case (MenuState::CREATE_WORLD):
			ImGui::InputText("World Name", m_WorldNameBuf, IM_ARRAYSIZE(m_WorldNameBuf));
			ImGui::Checkbox("Auto Seed", &m_AutoSeed);
			if (!m_AutoSeed)
			{
				ImGui::InputText("Seed", m_SeedBuf, IM_ARRAYSIZE(m_SeedBuf));
			}
			else
			{
				int randSeed = Magma::Random::Int(INT_MIN, INT_MAX);
				std::snprintf(m_SeedBuf, sizeof(m_SeedBuf), "%d", randSeed);
			}
			if (m_AutoSeed || strlen(m_SeedBuf) > 0) // catch non number seeds
			{
				if (ImGui::Button("Create"))
				{
					// initialize server
					m_NetworkManager->Begin();

					m_MenuState = MenuState::LOADING;
					// Create world
					std::string worldName = "New World";
					if (strlen(m_WorldNameBuf) > 0)
					{
						worldName = std::string(m_WorldNameBuf);
					}

					int seed;
					try 
					{
						seed = std::stoi(m_SeedBuf);
					}
					catch (std::invalid_argument)
					{
						seed = Magma::Random::Int(INT_MIN, INT_MAX);
					}
					m_NetworkManager->GetWorldManager()->CreateWorld(worldName, seed, *m_EntityWorld);
					m_MenuState = MenuState::IN_GAME; // FIX LOADING LATER
				}
			}
			if (ImGui::Button("Back"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::NONE);
				m_MenuState = MenuState::SINGLEPLAYER;
			}
			break;

		case (MenuState::LOAD_WORLD):
		{
			ImGui::InputText("World Path", m_WorldPathBuf, IM_ARRAYSIZE(m_WorldPathBuf));
			if (ImGui::Button("Browse"))
			{
				m_FileBrowser.Open();
			}

			m_FileBrowser.Display();

			if (m_FileBrowser.HasSelected())
			{
				std::memset(m_WorldPathBuf, 0, sizeof(m_WorldPathBuf));
				std::strncpy(m_WorldPathBuf, m_FileBrowser.GetSelected().string().c_str(), sizeof(m_WorldPathBuf));
				m_FileBrowser.ClearSelected();
			}

			if (strlen(m_WorldPathBuf) > 0)
			{
				if (ImGui::Button("Join"))
				{
					// initialize server
					m_NetworkManager->Begin();

					m_MenuState = MenuState::LOADING;

					if (!m_NetworkManager->GetWorldManager()->LoadWorld(m_WorldPathBuf))
					{
						ImGui::Text("Invalid file path.");
					}
					else
					{
						m_MenuState = MenuState::IN_GAME; // FIX LOADING LATER
					}
				}
			}
			if (ImGui::Button("Back"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::NONE);
				m_MenuState = MenuState::SINGLEPLAYER;
			}
			break;
		}
		case (MenuState::HOST_GAME):
			// Hosting options would go here
			if (ImGui::Button("Create World"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::SERVER);
				m_MenuState = MenuState::CREATE_WORLD;
			}
			if (ImGui::Button("Back"))
			{
				m_MenuState = MenuState::MULTIPLAYER;
			}
			break;

		case (MenuState::LOADING):
			ImGui::Text("Loading world...");
			// wont work until we have async loading
			break;
		case (MenuState::JOIN_GAME):
			// Joining options would go here
			m_NetworkManager->SetNetworkRole(Craft::NetworkRole::CLIENT);
			ImGui::Checkbox("Localhost", &m_AutoConnect);
			if (!m_AutoConnect)
			{
				ImGui::InputText("Server Address", m_ServerAddressBuf, IM_ARRAYSIZE(m_ServerAddressBuf));
				ImGui::InputText("Server Port", m_ServerportBuf, IM_ARRAYSIZE(m_ServerportBuf));
			}
			else
			{
				strcpy_s(m_ServerAddressBuf, "localhost");
				strcpy_s(m_ServerportBuf, "1233");
			}
			if ((m_AutoConnect || (strlen(m_ServerAddressBuf) > 0) && (strlen(m_ServerportBuf) > 0)))
			{
				if (ImGui::Button("Connect"))
				{
					m_NetworkManager->Begin();

					std::string address = std::string(m_ServerAddressBuf);
					enet_uint16 port = static_cast<enet_uint16>(std::stoi(std::string(m_ServerportBuf)));
					m_NetworkManager->GetClient()->SetServerHint(address.c_str(), port);
					m_NetworkManager->GetClient()->ConnectToServer();

					if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::FAILED)
					{
						ImGui::TextColored(ImVec4(1, 0, 0, 1), "Connection Timed Out!");
					}
				}
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::CONNECTING)
			{
				ImGui::Text("Connecting... %.1f s", m_NetworkManager->GetClient()->m_ConnectionTimer);
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::CONNECTED)
			{
				m_MenuState = MenuState::IN_GAME;
			}

			if (ImGui::Button("Back"))
			{
				m_MenuState = MenuState::MULTIPLAYER;
			}
			break;

		case (MenuState::IN_GAME):

			// In-game menu options would go here
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			glm::vec3 pos = m_EntityWorld->GetComponent<Craft::TransformComponent>(m_PrimaryCamera).worldMatrix[3];
			ImGui::Text("X: %.1f", pos[0]);
			ImGui::Text("Y: %.1f", pos[1]);
			ImGui::Text("Z: %.1f", pos[2]);
			
			if (m_NetworkManager->GetNetworkRole() == Craft::NetworkRole::SERVER)
			{
				if (ImGui::Button("Save & Exit"))
				{
					m_MenuState = MenuState::MAIN_MENU;
				}
			}
			else if (m_NetworkManager->GetNetworkRole() == Craft::NetworkRole::CLIENT)
			{
				if (ImGui::Button("Exit"))
				{
					m_MenuState = MenuState::MAIN_MENU;
				}
			}
			break;
	}
	ImGui::End();
}

void GameLayer::OnResize(int width, int height)
{
	// Resizing callback handling

	if (height == 0) height = 1;
	auto& camRef = m_EntityWorld->GetComponent<Craft::CameraComponent>(m_PrimaryCamera);
	camRef.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}
