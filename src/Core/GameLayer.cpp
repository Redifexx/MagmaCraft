#include <enet/enet.h>

#include <filesystem>
#include <limits.h>
#include <cstdio>

#include "Core/GameLayer.h"

#include "Core/Model.h"
#include "Core/ShaderProgram.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include "Core/AudioEngine.h"

#include "WorldManager.h"
#include "NetworkManager.h"
#include "BlockLibrary.h"
#include "Primitives.h"

#include "Datatypes/EntityWorld.h"
#include "Datatypes/Components/TransformComponent.h"
#include "Datatypes/Components/RelationshipComponent.h"
#include "Datatypes/Components/ModelComponent.h"
#include "Datatypes/Components/CameraComponent.h"
#include "Datatypes/Components/HealthComponent.h"
#include "Datatypes/Components/NativeScriptComponent.h"
#include "Datatypes/Components/PlayerComponent.h"

#include "Scripts/PlayerController.h"



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
	
	// setup systems & managers
	m_EntityWorld = std::make_shared<Craft::EntityWorld>();
	m_TransformSystem = std::make_unique<Craft::TransformSystem>();
	m_RenderSystem = std::make_unique<Craft::RenderSystem>();
	m_CameraSystem = std::make_unique<Craft::CameraSystem>();
	m_ScriptSystem = std::make_unique<Craft::ScriptSystem>();

	m_NetworkManager = std::make_shared<Craft::NetworkManager>();
	m_WorldStreamer = std::make_unique<Craft::WorldStreamer>(m_NetworkManager);
	m_NetworkManager->SetEntityWorld(m_EntityWorld);

	Craft::BlockLibrary::Initialize();

	int w, h;
	SDL_GetWindowSize(m_Window, &w, &h);

	// SHOULD BE MOVED INTO ANOTHER CLASS AT SOME POINT
	// Setup Framebuffers & Renderbuffers
	// resuing code from lava engine

	CreateScreenQuad(m_ScreenVAO, m_ScreenVBO);

	glGenFramebuffers(1, &m_ScreenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);

	// screen texture
	m_ScreenTextureColorBuffer = std::make_unique<Magma::Texture>(
		w, h, GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT, nullptr
	);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTextureColorBuffer->GetID(), 0);

	// create renderbuffer
	glGenRenderbuffers(1, &m_ScreenRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_ScreenRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ScreenRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "framebuffer error" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// move into resource manager later
	// Shader setup (Shader.h & ShaderProgram.h)
	// Mesh shader
	std::string vertpath = "resources/shaders/basic.vert";
	std::string fragpath = "resources/shaders/basic.frag";
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

	// screen quad shader
	vertpath = "resources/shaders/screenQuad.vert";
	fragpath = "resources/shaders/screenQuad.frag";
	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader screenVertexShader(vertpath, GL_VERTEX_SHADER);
	Shader screenFragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_ScreenShaderProgram = std::make_unique<ShaderProgram>();

	m_ScreenShaderProgram->AttachShader(screenVertexShader);
	m_ScreenShaderProgram->AttachShader(screenFragmentShader);
	if (!m_ScreenShaderProgram->Link())
	{
		std::cerr << "Failed to link shader program!" << std::endl;
		return;
	}


	// Single Texture setup
	m_Texture = std::make_unique<Texture>(texturePath.c_str(), true);
	m_Texture->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	m_Texture->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	m_ShaderProgram->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Texture->GetID());
	m_ShaderProgram->SetUniform("u_Texture", 0);

	// UITEST
	gl2d::init();
	m_UI = new glui::RendererUi();
	m_UIRenderer = new gl2d::Renderer2D();
	m_UIFont = new gl2d::Font();
	m_UITexture = new gl2d::Texture();
	m_UIRenderer->create();
	m_UIFont->createFromFile("resources/font/ANDYB.TTF");
	m_UITexture->loadFromFile("resources/textures/crosshair_shadow.png", true);
}

// ---- GAME UPDATE LOGIC ----
void GameLayer::OnUpdate(float dt)
{
	if (dt > 0.1f) dt = 0.1f; // safaty

	int w, h;
	SDL_GetWindowSize(m_Window, &w, &h);

	//clear screen
	glViewport(0, 0, w, h);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
	glClearColor(0.643f, 0.827f, 0.984f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Mouse look
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse && Magma::Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
	{
		SDL_SetWindowRelativeMouseMode(m_Window, true);
	}

	if (m_IsLocalPlayerLoaded)
	{
		m_ScriptSystem->Update(*m_EntityWorld, dt);

		if (m_AutoSaveTimer <= 0.0f)
		{
			m_AutoSaveTimer = m_AutoSaveRate;
			m_NetworkManager->GetWorldManager().get()->SaveWorld(*m_EntityWorld);
		}
		else
		{
			m_AutoSaveTimer -= dt;
		}

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


		// ---- WORLD UPDATE ---- FINISH
		m_WorldStreamer->Update(dt, glm::vec3(camTransform.worldMatrix[3]));

		// ---- AUDIO UPDATE ----
		// Audio Listener Update
		Magma::AudioEngine::UpdateListener(glm::vec3(camTransform.worldMatrix[3]), -glm::vec3(camTransform.worldMatrix[2]), glm::vec3(camTransform.worldMatrix[1]));

		// --- PLAYER DATA INTERPOLATION ---
		auto* playerPool = m_EntityWorld->GetComponentPool<Craft::PlayerComponent>();
		auto* transformPool = m_EntityWorld->GetComponentPool<Craft::TransformComponent>();

		if (playerPool && transformPool)
		{
			for (auto entity : playerPool->GetAllEntities())
			{
				// invalid entities
				if (!transformPool->Contains(entity)) continue;

				auto& playerRef = playerPool->Get(entity);

				if (playerRef.isLocalPlayer) continue; // skip local player
				if (playerRef.interpolationTime >= playerRef.interpolationDuration) continue; // skip if already at target

				auto& transformRef = transformPool->Get(entity);
				playerRef.interpolationTime += dt;

				float t = playerRef.interpolationTime / playerRef.interpolationDuration;
				if (t > 1.0f) t = 1.0f;

				transformRef.localPosition = glm::mix(playerRef.startPos, playerRef.targetPos, t);
				transformRef.localRotation = glm::slerp(playerRef.startRot, playerRef.targetRot, t);

				transformRef.isDirty = true;
			}
		}

		// --- TRANSFORMS UPDATE ----
		m_TransformSystem->Update(*m_EntityWorld);
		m_CameraSystem->Update(*m_EntityWorld);

		// ---- RENDERING ----
		// Shader uniforms update and model drawing
		m_ShaderProgram->Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Texture->GetID());
		m_ShaderProgram->SetUniform("u_Texture", 0);
	}

	// network tick
	m_NetworkTickTimer += dt;

	while (m_NetworkTickTimer >= m_NetworkTickRate)
	{
		m_NetworkTickTimer = 0.0f;

		// local player is only loaded when logged in
		if (m_IsLocalPlayerLoaded && m_NetworkManager->GetNetworkRole() != Craft::NetworkRole::NONE)
		{
			auto& transformRef = m_EntityWorld->GetComponent<Craft::TransformComponent>(m_Player);
			auto& playerRef = m_EntityWorld->GetComponent<Craft::PlayerComponent>(m_Player);

			// set velocity here

			m_NetworkManager->SendPlayerData(*m_EntityWorld, m_Player);
		}
	}

	m_RenderSystem->Render(*m_EntityWorld, *m_ShaderProgram, m_WorldStreamer.get(), m_Window);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	m_ScreenShaderProgram->Use();

	glBindVertexArray(m_ScreenVAO);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, m_ScreenTextureColorBuffer->GetID());
	glDrawArrays(GL_TRIANGLES, 0, 6);

	RenderUI(w, h, dt);

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

	// double clear need to fix
	m_NetworkManager->GetWorldManager().get()->SaveWorld(*m_EntityWorld);

	m_WorldStreamer->UnloadAllChunks();
	m_WorldStreamer.reset();

	m_NetworkManager->End();

	delete m_UI;
	delete m_UIRenderer;
	delete m_UIFont;
	delete m_UITexture;
	glDeleteFramebuffers(1, &m_ScreenFBO);
	glDeleteRenderbuffers(1, &m_ScreenRBO);

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

void GameLayer::OnImGuiRender(float dt)
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
		case (MenuState::SET_NAME):
			ImGui::InputText("Username", m_UserNameBuf, IM_ARRAYSIZE(m_UserNameBuf));
			if (strlen(m_UserNameBuf) > 0)
			{
				if (ImGui::Button("Play Game"))
				{
					m_Username = std::string(m_UserNameBuf);
					m_NetworkManager->SetLocalPlayerUsername(m_Username); // important for login packets
					m_MenuState = MenuState::MAIN_MENU;
				}
			}
			if (ImGui::Button("Exit"))
			{
				SDL_Event quitEvent;
				quitEvent.type = SDL_EVENT_QUIT;
				SDL_PushEvent(&quitEvent);
			}
			break;

		case (MenuState::MAIN_MENU):
			if (ImGui::Button("Singleplayer"))
			{
				m_MenuState = MenuState::SINGLEPLAYER;
			}
			if (ImGui::Button("Multiplayer"))
			{
				m_MenuState = MenuState::MULTIPLAYER;
			}
			if (ImGui::Button("Back"))
			{
				m_MenuState = MenuState::SET_NAME;
			}
			break;

		case (MenuState::SINGLEPLAYER):
			if (ImGui::Button("Create World"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::SERVER);
				// initialize server
				m_NetworkManager->Begin();
				m_MenuState = MenuState::CREATE_WORLD;
			}
			if (ImGui::Button("Load World"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::SERVER);
				// initialize server
				m_NetworkManager->Begin();
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
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::CLIENT);
				m_NetworkManager->Begin();
			}
			if (ImGui::Button("Back"))
			{
				m_MenuState = MenuState::MAIN_MENU;
			}
			break;

		case (MenuState::CREATE_WORLD):
		{
			if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::DISCONNECTED)
			{
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
						m_NetworkManager->GetWorldManager().get()->CreateWorld(worldName, seed, *m_EntityWorld);

						// Connect to ourselves
						strcpy_s(m_ServerAddressBuf, "localhost");
						strcpy_s(m_ServerportBuf, "1233");

						std::string address = std::string(m_ServerAddressBuf);
						enet_uint16 port = static_cast<enet_uint16>(std::stoi(std::string(m_ServerportBuf)));
						m_NetworkManager->GetClient()->SetServerHint(address.c_str(), port);
						m_NetworkManager->GetClient()->ConnectToServer();

						m_ConnectionFailTimer = m_ConnectionFailRate;
					}
				}
				if (ImGui::Button("Back"))
				{
					m_NetworkManager->End();
					m_MenuState = MenuState::MAIN_MENU;
				}
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::CONNECTING)
			{
				ImGui::Text("Connecting... %.1f s", m_NetworkManager->GetClient()->m_ConnectionTimer);
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::AUTHENTICATING)
			{
				ImGui::Text("Logging In...");
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::LOGGED_IN)
			{
				m_MenuState = MenuState::IN_GAME;

				// spawn player
				SpawnLocalPlayer();
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::FAILED) // shouldn't occur on localhost
			{
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "Connection Timed Out!");
				m_ConnectionFailTimer -= dt;

				if (m_ConnectionFailTimer <= 0.0f)
				{
					m_NetworkManager->GetClient()->SetConnectionState(ConnectionState::DISCONNECTED);
					m_ConnectionFailTimer = 0.0f;
				}
			}

			break;
		}
		case (MenuState::LOAD_WORLD):
		{
			if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::DISCONNECTED)
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
						if (!m_NetworkManager->GetWorldManager().get()->LoadWorld(m_WorldPathBuf))
						{
							ImGui::Text("Invalid file path.");
							return;
						}

						// Connect to ourselves
						strcpy_s(m_ServerAddressBuf, "localhost");
						strcpy_s(m_ServerportBuf, "1233");

						std::string address = std::string(m_ServerAddressBuf);
						enet_uint16 port = static_cast<enet_uint16>(std::stoi(std::string(m_ServerportBuf)));
						m_NetworkManager->GetClient()->SetServerHint(address.c_str(), port);
						m_NetworkManager->GetClient()->ConnectToServer();

						m_ConnectionFailTimer = m_ConnectionFailRate;
					}
				}

				if (ImGui::Button("Back"))
				{
					m_NetworkManager->End();
					m_MenuState = MenuState::MAIN_MENU;
				}
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::CONNECTING)
			{
				ImGui::Text("Connecting... %.1f s", m_NetworkManager->GetClient()->m_ConnectionTimer);
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::AUTHENTICATING)
			{
				ImGui::Text("Logging In...");
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::LOGGED_IN)
			{
				m_MenuState = MenuState::IN_GAME;

				// spawn player
				SpawnLocalPlayer();
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::FAILED) // shouldn't occur on localhost
			{
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "Connection Timed Out!");
				m_ConnectionFailTimer -= dt;

				if (m_ConnectionFailTimer <= 0.0f)
				{
					m_NetworkManager->GetClient()->SetConnectionState(ConnectionState::DISCONNECTED);
					m_ConnectionFailTimer = 0.0f;
				}
			}
			break;
		}
		case (MenuState::HOST_GAME):
			// Hosting options would go here
			if (ImGui::Button("Create World"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::SERVER);
				m_NetworkManager->Begin();
				m_MenuState = MenuState::CREATE_WORLD;
			}
			if (ImGui::Button("Load World"))
			{
				m_NetworkManager->SetNetworkRole(Craft::NetworkRole::SERVER);
				m_NetworkManager->Begin();
				m_MenuState = MenuState::LOAD_WORLD;
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
			// Join Server -> Connect -> Login -> Enter World

			if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::DISCONNECTED)
			{
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
						std::string address = std::string(m_ServerAddressBuf);
						enet_uint16 port = static_cast<enet_uint16>(std::stoi(std::string(m_ServerportBuf)));
						m_NetworkManager->GetClient()->SetServerHint(address.c_str(), port);
						m_NetworkManager->GetClient()->ConnectToServer();

						m_ConnectionFailTimer = m_ConnectionFailRate;
					}
				}
				if (ImGui::Button("Back"))
				{
					m_NetworkManager->End();
					m_MenuState = MenuState::MULTIPLAYER;
				}
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::CONNECTING)
			{
				ImGui::Text("Connecting... %.1f s", m_NetworkManager->GetClient()->m_ConnectionTimer);
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::AUTHENTICATING)
			{
				ImGui::Text("Logging In...");
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::LOGGED_IN)
			{
				m_MenuState = MenuState::IN_GAME;

				// spawn player
				SpawnLocalPlayer();
			}
			else if (m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::FAILED)
			{
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "Connection Timed Out!");
				m_ConnectionFailTimer -= dt;

				if (m_ConnectionFailTimer <= 0.0f)
				{
					m_NetworkManager->GetClient()->SetConnectionState(ConnectionState::DISCONNECTED);
					m_ConnectionFailTimer = 0.0f;
				}
			}

			break;

		case (MenuState::IN_GAME):

			// In-game menu options would go here
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text("Autosave in %.1f seconds", m_AutoSaveTimer);
			if (m_Player != Craft::NULL_ENTITY &&
				m_EntityWorld->HasEntityID(m_Player) &&
				m_EntityWorld->Contains<Craft::TransformComponent>(m_Player))
			{
				glm::vec3 pos = m_EntityWorld->GetComponent<Craft::TransformComponent>(m_Player).worldMatrix[3];
				ImGui::Text("X: %.1f", pos[0]);
				ImGui::Text("Y: %.1f", pos[1]);
				ImGui::Text("Z: %.1f", pos[2]);
			}
			else
			{
				ImGui::Text("Camera: Detached");
			}

			ImGui::Text("Players:");
			for (std::string& name : m_NetworkManager->m_PlayerNames)
			{
				ImGui::Text("%s", name.c_str());
			}
			
			if (m_NetworkManager->GetNetworkRole() == Craft::NetworkRole::SERVER)
			{
				if (ImGui::Button("Save & Exit"))
				{
					m_NetworkManager->GetWorldManager().get()->SaveWorld(*m_EntityWorld);
					m_NetworkManager->ShutdownServer();
					CleanupLocalPlayer();
					m_WorldStreamer->UnloadAllChunks();
					m_EntityWorld->ClearAllEntities();
					m_NetworkManager->End();
					m_MenuState = MenuState::MAIN_MENU;
				}
			}
			else if (m_NetworkManager->GetNetworkRole() == Craft::NetworkRole::CLIENT)
			{
				if (ImGui::Button("Disconnect") || m_NetworkManager->GetClient()->GetConnectionState() == ConnectionState::DISCONNECTED)
				{
					m_NetworkManager->DisconnectFromServer();
					CleanupLocalPlayer();
					m_WorldStreamer->UnloadAllChunks();
					m_EntityWorld->ClearAllEntities();
					m_NetworkManager->End();
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
	if (m_PrimaryCamera != Craft::NULL_ENTITY)
	{
		auto& camRef = m_EntityWorld->GetComponent<Craft::CameraComponent>(m_PrimaryCamera);
		camRef.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	}

	// resize screen quad texture
	glBindTexture(GL_TEXTURE_2D, m_ScreenTextureColorBuffer->GetID());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, m_ScreenRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void GameLayer::SpawnLocalPlayer()
{
	// create player using world manager
	uint32_t playerEntity = m_EntityWorld->m_PlayerIDEntityMap[m_NetworkManager->GetNetworkID()];

	// set local player flag
	auto& playerRef = m_EntityWorld->GetComponent<Craft::PlayerComponent>(playerEntity);
	playerRef.isLocalPlayer = true; // here
	m_Player = playerEntity;

	auto& transformRef = m_EntityWorld->GetComponent<Craft::TransformComponent>(playerEntity);
	std::cout << "Local Player Spawned in at: " << transformRef.localPosition.x;
	std::cout << " " << transformRef.localPosition.y;
	std::cout << " " << transformRef.localPosition.z << std::endl;

	// add camera
	uint32_t cameraEntity = m_EntityWorld->AddEntity();
	m_EntityWorld->AddComponent<Craft::TransformComponent>(cameraEntity, {
		glm::vec3(0.0f, 1.62f, -0.05f),         
		glm::quat(1.0f, 0.0f, 0.0f, 0.0f),   
		glm::vec3(1.0f)                      
	});
	m_EntityWorld->AddComponent<Craft::CameraComponent>(cameraEntity, { true });
	m_PrimaryCamera = cameraEntity;

	// make camera a child of player
	auto& playerRelRef = m_EntityWorld->GetComponent<Craft::RelationshipComponent>(playerEntity);

	playerRelRef =
	{
		Craft::NULL_ENTITY,
		cameraEntity,
		Craft::NULL_ENTITY,
		Craft::NULL_ENTITY
	};

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

	// set script window / inject dependencies
	SDL_Window* windowPtr = m_Window;
	scriptComponent.instantiateScript = [windowPtr]()
	{
		auto* controller = new Craft::PlayerController();
		controller->SetWindow(windowPtr);
		return static_cast<Craft::ScriptableEntity*>(controller);
	};

	m_EntityWorld->SetLocalPlayerID(playerEntity);
	m_IsLocalPlayerLoaded = true;
	m_AutoSaveTimer = m_AutoSaveRate;
}

void GameLayer::CleanupLocalPlayer()
{
	if (m_Player != Craft::NULL_ENTITY && m_EntityWorld->HasEntityID(m_Player))
	{
		m_EntityWorld->RemoveEntity(m_Player);
	}
	m_Player = Craft::NULL_ENTITY;
	m_PrimaryCamera = Craft::NULL_ENTITY;
	m_EntityWorld->SetLocalPlayerID(Craft::NULL_ENTITY);
	m_IsLocalPlayerLoaded = false;
	m_AutoSaveTimer = 0.0f;
}

void GameLayer::WorldShutdown()
{
	m_NetworkManager->GetWorldManager().get()->SaveWorld(*m_EntityWorld);
	m_WorldStreamer->UnloadAllChunks();
	m_NetworkManager->End();
	CleanupLocalPlayer();
}

// this just renders the crosshair for now
void GameLayer::RenderUI(const int& w, const int& h, float dt)
{
	m_UIRenderer->updateWindowMetrics(w, h);

	if (m_IsLocalPlayerLoaded)
	{
		float size = 8.0f;
		float x = (w / 2.0f) - (size / 2.0f);
		float y = (h / 2.0f) - (size / 2.0f);
		gl2d::Rect rect = { x, y, size, size };
		m_UIRenderer->renderRectangle(rect, *m_UITexture, Colors_White);
	}

	m_UI->renderFrame(*m_UIRenderer, *m_UIFont, Magma::Input::GetMousePosition(),
		Magma::Input::IsMouseButtonPressed(SDL_BUTTON_LEFT), Magma::Input::IsMouseButtonHeld(SDL_BUTTON_LEFT), Magma::Input::IsMouseButtonReleased(SDL_BUTTON_LEFT),
		Magma::Input::IsKeyReleased(SDL_SCANCODE_ESCAPE), "", dt, 0);

	m_UIRenderer->flush();
}

void GameLayer::CreateScreenQuad(unsigned int& vao, unsigned int& vbo)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Magma::quadVertices), Magma::quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}