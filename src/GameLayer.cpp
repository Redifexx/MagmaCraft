#include <enet/enet.h>
#include "GameLayer.h"

#include "Model.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include <filesystem>
#include <AudioEngine.h>
#include "WorldManager.h"
#include "NetworkManager.h"
#include "BlockLibrary.h"
#include <limits.h>

using namespace Magma;

void GameLayer::OnAttach()
{
	// ---- NETWORK INITIALIZATION ----

	if (enet_initialize() != 0)
	{
		std::cout << "Error initializing enet...";
		return;
	}

	// ---- GAME INITIALIZATION ----

	// Model setup (Model.h)
	// make model read raw vertices
	m_ModelMatrix = glm::mat4(1.0f);

	// Shader setup (Shader.h & ShaderProgram.h)
	std::string vertpath = "resources/shaders/basic.vert";
	std::string fragpath = "resources/shaders/basic.frag";

	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
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

	// Camera setup (Camera.h)
	m_Camera = std::make_unique<Camera>(glm::vec3(0.0f, 64.0f, 0.0f));
	m_Camera->SetPerspective(true);

	// Initial shader uniforms setup
	m_ShaderProgram->Use();
	m_ShaderProgram->SetUniform("u_Model", m_ModelMatrix);
	m_ShaderProgram->SetUniform("u_ViewProjection", m_Camera->GetViewProjectionMatrix());

	// Network Manager Setup
	m_NetworkManager = std::make_shared<Craft::NetworkManager>();
	m_WorldStreamer = std::make_unique<Craft::WorldStreamer>(m_NetworkManager);

	//BlockLibrary Setup
	Craft::BlockLibrary::Initialize();
	


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_Texture->GetID());
	//m_ShaderProgram->SetUniform("u_Texture", 0);
}

// ---- GAME UPDATE LOGIC ----
void GameLayer::OnUpdate(float dt)
{
	// ---- INPUT ----
	// Basic input handling for Camera movement (Input.h)
	// Should probably be handled by a manager class
	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_W))
		m_Camera->SetPosition(m_Camera->GetPosition() + m_Camera->GetFront() * 5.0f * dt);

	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_S))
		m_Camera->SetPosition(m_Camera->GetPosition() + m_Camera->GetFront() * -5.0f * dt);

	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_A))
		m_Camera->SetPosition(m_Camera->GetPosition() + m_Camera->GetRight() * -5.0f * dt);

	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_D))
		m_Camera->SetPosition(m_Camera->GetPosition() + m_Camera->GetRight() * 5.0f * dt);

	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_E))
		m_Camera->SetPosition(m_Camera->GetPosition() + m_Camera->GetUp() * 5.0f * dt);

	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_Q))
		m_Camera->SetPosition(m_Camera->GetPosition() + m_Camera->GetUp() * -5.0f * dt);

	if (Magma::Input::IsKeyPressed(SDL_SCANCODE_F))
	{
		//Magma::AudioEngine::PlayGlobal("R:/Code/Magma/resources/audio/wind.mp3", 0.1f, true);
		//Magma::AudioEngine::PlayAtLocation("resources/audio/pickitup.mp3", glm::vec3(0.0f, 0.0f, 0.0f), 1.5f, true);
	}

	if (Magma::Input::IsKeyPressed(SDL_SCANCODE_G))
		Magma::AudioEngine::StopGlobal();

	// Mouse look
	ImGuiIO& io = ImGui::GetIO();
	// Setup Window Cursor Lock
	if (!io.WantCaptureMouse && Magma::Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
	{
		SDL_SetWindowRelativeMouseMode(m_Window, true);
	}

	if (SDL_GetWindowRelativeMouseMode(m_Window)) // check if cursor is captured
	{
		float mouseX = Magma::Input::GetMouseDelta().x;
		float mouseY = Magma::Input::GetMouseDelta().y;

		float camYaw = m_Camera->GetYaw() + mouseX * 0.1f;
		float camPitch = m_Camera->GetPitch() - mouseY * 0.1f;
		camPitch = glm::clamp(camPitch, -89.0f, 89.0f);
		m_Camera->SetYaw(camYaw);
		m_Camera->SetPitch(camPitch);
		m_Camera->UpdateCameraVectors();
	}


	// ---- WORLD UPDATE ---- FINISH
	m_WorldStreamer->Update(dt, m_Camera->GetPosition());

	// ---- AUDIO UPDATE ----
	// Audio Listener Update
	Magma::AudioEngine::UpdateListener(m_Camera->GetPosition(), m_Camera->GetFront(), m_Camera->GetUp());

	// ---- RENDERING ----
	// Shader uniforms update and model drawing
	m_ShaderProgram->Use();
	m_ShaderProgram->SetUniform("u_ViewProjection", m_Camera->GetViewProjectionMatrix());


	m_WorldStreamer->GetWorldRenderer()->DrawWorld();

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
	delete m_ShaderProgram;
	delete m_Camera;
	delete m_Texture;
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
	ImGui::TextUnformatted(logoArt);
	ImGui::Text("by Gio Perez Colon");
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
				m_SeedBuf = Magma::Random::Int(INT_MIN, INT_MAX);
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
					m_NetworkManager->GetWorldManager()->CreateWorld(worldName, std::stoi(static_cast<std::string>(m_SeedBuf)));
					m_MenuState = MenuState::IN_GAME; // FIX LOADING LATER
				}
			}
			if (ImGui::Button("Back"))
			{
				m_MenuState = MenuState::SINGLEPLAYER;
			}
			break;

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
			if ((m_AutoConnect || (IM_ARRAYSIZE(m_ServerAddressBuf) > 0) && (IM_ARRAYSIZE(m_ServerportBuf) > 0)) && m_NetworkManager->GetClient()->GetConnectionState() != ConnectionState::CONNECTING)
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
			glm::vec3 pos = m_Camera->GetPosition();
			ImGui::Text("X: %.1f", pos[0]);
			ImGui::Text("Y: %.1f", pos[1]);
			ImGui::Text("Z: %.1f", pos[2]);
			break;
	}
	ImGui::End();
}

void GameLayer::OnResize(int width, int height)
{
	// Resizing callback handling

	if (height == 0) height = 1;
	m_Camera->SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}
