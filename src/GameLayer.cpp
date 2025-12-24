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

	while (true)
	{
		std::cout << "S_erver or C_lien?: ";
		char c = 0;
		if (std::cin >> c)
		{
			if (std::tolower(c) == 's')
			{
				ServerFunction();
				break;
			}
			if (std::tolower(c) == 'c')
			{
				ClientFunction();
				break;
			}
		}
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

	m_Host = true;
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
	ImGui::Checkbox("Server Host?", &m_Host);
	if (ImGui::Button("Start Network"))
	{
		//does nothing yet
	}
	ImGui::End();
}

void GameLayer::OnResize(int width, int height)
{
	// Resizing callback handling

	if (height == 0) height = 1;
	m_Camera->SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}

// Thank you Low Level Game Dev
void GameLayer::ServerFunction()
{
	ENetAddress address = {};
	ENetHost* server = nullptr;
	address.host = ENET_HOST_ANY; // takes clients from any IP address
	address.port = 1233; // check ports later

	server = enet_host_create(&address /* the address to bind the server host to */,
		32 /* allow up to 32 clients and/or outgoing connections */,
		2 /* allow up to 2 channels to be used, 0 and 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */
	);

	if (server == NULL)
	{
		std::cout << "Problem creating server\n";
		return;
	}

	ENetPeer* client = 0;

	// server game loop
	while (true)
	{
		// receive events

		ENetEvent event = {};
		/* Wait up to 10 ms for an event */
		// you probably don't want to wait here in a game and do other stuff,
		// or calculate how much you can wait each time
		while (enet_host_service(server, &event, 10) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
					printf("A new client connected from %x:%u.\n",
						event.peer->address.host,
						event.peer->address.port);

					client = event.peer;

					break;

				case ENET_EVENT_TYPE_RECEIVE:
					printf("A packet of length %u containing %s was received on channel %u.\n",
						event.packet->dataLength,
						event.packet->data,
						event.channelID);
					/* clean up the packet after usage */
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					printf("client disconnected.\n");

					// should also check on which client disconnected
					// assuming 1 client for example
					client = 0;

					break;
			}
		}

		if (client)
		{
			std::string message = "";
			std::cin >> message;
			 
			if (message != "")
			{
				SendMessage(message.c_str(), message.size() + 1, client);
			}
		}
	}

	enet_host_destroy(server);
}

void GameLayer::ClientFunction()
{
	ENetHost* client = nullptr;
	ENetPeer* server = nullptr;

	client = enet_host_create(NULL /* CREATE A CLIENT HOST */,
		1 /* only allow outgoing connection */,
		2 /* allow up to 2 channels to be used, 0 & 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */
	);

	if (client == NULL)
	{
		std::cout << "An error occurred while trying to create an ENetclient host.\n";
		return;
	}

	ENetAddress address = {};
	//enet_address_set_host(&address, "127.0.0.1");
	enet_address_set_host(&address, "localhost");
	address.port = 1233;

	server = enet_host_connect(client, &address, 2, 0);

	if (server == nullptr)
	{
		std::cout << "Wasn't able to initialize connection\n";

		// don't forget
		enet_host_destroy(client);
	}
	else
	{
		/* Wait up to 5 seconds for the conection attempt to succeed */
		ENetEvent event = {};
		if (enet_host_service(client, &event, 5000) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT)
		{
			std::cout << "Got a connection!\n";
			while (true)
			{

			}
		}
		else
		{
			std::cout << "Wasn't able to connect\n";
		}

		enet_peer_reset(server);
		enet_host_destroy(client);
	}
}

void GameLayer::SendMessage(const char* data, size_t s, ENetPeer* to)
{
	ENetPacket* packet = enet_packet_create(data, s, ENET_PACKET_FLAG_RELIABLE);

	// the second parameter is the channel id
	enet_peer_send(to, 0, packet);
}