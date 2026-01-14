#include <enet/enet.h>
#include "GameLayer.h"

#include "Model.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include <filesystem>
#include <AudioEngine.h>

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
	m_Models.push_back(new Model("resources/models/radio.fbx"));
	m_ModelMatrix = glm::mat4(1.0f);
	m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
	m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// Shader setup (Shader.h & ShaderProgram.h)
	std::string vertpath = "resources/shaders/basic.vert";
	std::string fragpath = "resources/shaders/basic.frag";

	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader vertexShader(vertpath, GL_VERTEX_SHADER);
	Shader fragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_ShaderProgram = new ShaderProgram();

	m_ShaderProgram->AttachShader(vertexShader);
	m_ShaderProgram->AttachShader(fragmentShader);
	if (!m_ShaderProgram->Link())
	{
		std::cerr << "Failed to link shader program!" << std::endl;
		return;
	}
	
	// Texture setup
	m_Texture = new Texture("resources/textures/radio.png");
	m_Texture->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_Texture->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Camera setup (Camera.h)
	m_Camera = new Camera(glm::vec3(0.0f, 0.0f, 5.0f));
	m_Camera->SetPerspective(true);

	// Initial shader uniforms setup
	m_ShaderProgram->Use();
	m_ShaderProgram->SetUniform("u_Model", m_ModelMatrix);
	m_ShaderProgram->SetUniform("u_ViewProjection", m_Camera->GetViewProjectionMatrix());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Texture->GetID());
	m_ShaderProgram->SetUniform("u_Texture", 0);
}

void GameLayer::OnUpdate(float dt)
{
	// ---- GAME UPDATE LOGIC ----


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
		Magma::AudioEngine::PlayGlobal("R:/Code/Magma/resources/audio/wind.mp3", 0.1f, true);
		Magma::AudioEngine::PlayAtLocation("resources/audio/pickitup.mp3", glm::vec3(0.0f, 0.0f, 0.0f), 1.5f, true);
	}

	if (Magma::Input::IsKeyPressed(SDL_SCANCODE_G))
		Magma::AudioEngine::StopGlobal();

	// Mouse look
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

	// Audio Listener Update
	Magma::AudioEngine::UpdateListener(m_Camera->GetPosition(), m_Camera->GetFront(), m_Camera->GetUp());

	// Shader uniforms update and model drawing
	m_ShaderProgram->Use();
	m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(40.0f * dt), glm::vec3(0.0f, 0.0f, 1.0f));
	m_ShaderProgram->SetUniform("u_Model", m_ModelMatrix);
	m_ShaderProgram->SetUniform("u_ViewProjection", m_Camera->GetViewProjectionMatrix());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Texture->GetID());
	m_ShaderProgram->SetUniform("u_Texture", 0);

	for (Model* model : m_Models)
	{
		model->Draw();
	}

	Magma::Input::Update();
	Magma::AudioEngine::UpdateActiveSounds();

	// Network Update Logic
	if (m_NetworkInitialized && m_Host)
	{
		m_Server->Update();
	}

	if (m_NetworkInitialized && !m_Host)
	{
		m_Client->Update();
		if (m_ConnectionState == ConnectionState::CONNECTING)
		{
			if (m_Client->IsConnected())
			{
				m_ConnectionState = ConnectionState::CONNECTED;
			}
			else
			{
				m_ConnectionTimer -= dt;
				if (m_ConnectionTimer <= 0.0f)
				{
					m_ConnectionState = ConnectionState::FAILED;
				}
			}
		}
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

void GameLayer::OnImGuiRender()
{
	// --- IMGUI RENDERING ----
	static char buf[256] = "";
	ImGui::Begin("Sample Debug Menu");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Text("Use WASD to nagivate.");
	ImGui::Text("Use E, Q to move up and down.");
	ImGui::Text("Use F to begin audio.");
	ImGui::Text("Use G to end audio.");

	if (!m_NetworkInitialized)
	{
		if (ImGui::Button("Start Server"))
		{
			m_Server = new Server();
			m_Host = true;
			m_NetworkInitialized = true;
		}
		if (ImGui::Button("Start Client"))
		{
			m_Client = new Client();
			m_Host = false;
			m_NetworkInitialized = true;
		}
	}
	else
	{
		if (!m_Host)
		{
			ImGui::Text("CLIENT");
			if ((m_ConnectionState == ConnectionState::DISCONNECTED) || (m_ConnectionState == ConnectionState::FAILED))
			{
				ImGui::Checkbox("Auto", &m_AutoConnect);
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
				if (ImGui::Button("Connect"))
				{
					std::string address = std::string(m_ServerAddressBuf);
					enet_uint16 port = static_cast<enet_uint16>(std::stoi(std::string(m_ServerportBuf)));
					m_Client->SetServerHint(address.c_str(), port);

					if (m_Client->ConnectToServer())
					{
						m_ConnectionState = ConnectionState::CONNECTING;
						m_ConnectionTimer = CONNECTION_TIMEOUT;
					}

					if (m_ConnectionState == ConnectionState::FAILED)
					{
						ImGui::TextColored(ImVec4(1, 0, 0, 1), "Connection Timed Out!");
					}
				}
			}
			else if (m_ConnectionState == ConnectionState::CONNECTING)
			{
				ImGui::Text("Connecting... %.1f s", m_ConnectionTimer);
			}
			else if (m_ConnectionState == ConnectionState::CONNECTED)
			{
				ImGui::Text("Connected to %s:%s", m_ServerAddressBuf, m_ServerportBuf);

				if (ImGui::BeginChild("Chat", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar))
				{
					for (const auto& msg : m_Client->m_MessageBuffer)
					{
						ImGui::TextWrapped("%s", msg.c_str());
					}

					if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
					{
						ImGui::SetScrollHereY(1.0f);
					}

					ImGui::EndChild();
				}
				ImGui::InputText("Msg", m_NetworkMsg, IM_ARRAYSIZE(m_NetworkMsg));
				if (ImGui::Button("Send"))
				{
					m_Client->SendPacket(m_NetworkMsg, true);
				}

			}
		}
		else
		{
			ImGui::Text("SERVER");

			ImGui::Text("[%d/%d] Users", m_Server->GetClientCount(), m_Server->GetMaxClients());

			if (ImGui::BeginChild("Chat", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar))
			{
				if (!m_Server->m_MessageBuffer.empty())
				{
					for (const auto& msg : m_Server->m_MessageBuffer)
					{
						ImGui::TextWrapped("%s", msg.c_str());
					}
				}

				if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				{
					ImGui::SetScrollHereY(1.0f);
				}

				ImGui::EndChild();
			}
			ImGui::InputText("Msg", m_NetworkMsg, IM_ARRAYSIZE(m_NetworkMsg));
			if (ImGui::Button("Send"))
			{
				m_Server->SendPacket(m_NetworkMsg, true);
			}

		}
	}
	ImGui::End();
}

void GameLayer::OnResize(int width, int height)
{
	// Resizing callback handling

	if (height == 0) height = 1;
	m_Camera->SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}
