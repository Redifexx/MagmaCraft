#include <enet/enet.h>

#include <filesystem>
#include <limits.h>
#include <cstdio>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "Core/GameLayer.h"

#include "Core/Model.h"
#include "Core/ShaderProgram.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include "Core/AudioEngine.h"

#include "WorldManager.h"
#include "WorldSelector.h"
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

//#include <tracy/Tracy.hpp>

// refactor needed, this file has become a monolith

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
	m_WorldSelector = std::make_unique<Craft::WorldSelector>();

	Craft::BlockLibrary::Initialize();

	int w, h;
	SDL_GetWindowSize(m_Window, &w, &h);

	// SHOULD BE MOVED INTO ANOTHER CLASS AT SOME POINT
	// Setup Framebuffers & Renderbuffers
	// resuing code from lava engine

	CreateCube();
	CreateScreenQuad(m_ScreenVAO, m_ScreenVBO);
	

	glGenFramebuffers(1, &m_ScreenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);

	// screen texture
	m_ScreenTextureColorBuffer = std::make_unique<Magma::Texture>(
		w, h, GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT, nullptr
	);
	m_ScreenTextureColorBuffer->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_ScreenTextureColorBuffer->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTextureColorBuffer->GetID(), 0);

	// create renderbuffer
	glGenRenderbuffers(1, &m_ScreenRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_ScreenRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ScreenRBO);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "framebuffer error" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Setup GBuffer
	glGenFramebuffers(1, &m_GBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer);

	// pos buffer
	m_GPosition = std::make_unique<Magma::Texture>(
		w, h, GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr
	);
	m_GPosition->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_GPosition->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GPosition->GetID(), 0);

	// normal buffer
	m_GNormal = std::make_unique<Magma::Texture>(
		w, h, GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr
	);
	m_GNormal->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_GNormal->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GNormal->GetID(), 0);

	// color buffer
	m_GAlbedo = std::make_unique<Magma::Texture>(
		w, h, GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
	);
	m_GAlbedo->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_GAlbedo->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GAlbedo->GetID(), 0);

	// ASME material map buffer
	m_GMatData = std::make_unique<Magma::Texture>(
		w, h, GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
	);
	m_GMatData->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_GMatData->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GMatData->GetID(), 0);

	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	// setup depth texture

	m_GDepth = std::make_unique<Magma::Texture>(
		w, h, GL_TEXTURE_2D, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr
	);
	m_GDepth->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_GDepth->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_GDepth->GetID(), 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "framebuffer error" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// lighting pass fbo
	glGenFramebuffers(1, &m_GLightingPassFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GLightingPassFBO);
	m_GLightingPass = std::make_unique<Magma::Texture>(
		w, h, GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT, nullptr
	);
	m_GLightingPass->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_GLightingPass->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GLightingPass->GetID(), 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GDepth->GetID(), 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "framebuffer error" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	// move into resource manager later
	// Shader setup (Shader.h & ShaderProgram.h)
	// Mesh shader
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
		std::cerr << "Failed to link geometry shader program!" << std::endl;
		return;
	}

	// lighting pass shader
	vertpath = "resources/shaders/screenQuad.vert";
	fragpath = "resources/shaders/lighting.frag";
	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader quadVertexShader(vertpath, GL_VERTEX_SHADER);
	Shader lightingFragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_LightingShaderProgram = std::make_unique<ShaderProgram>();

	m_LightingShaderProgram->AttachShader(quadVertexShader);
	m_LightingShaderProgram->AttachShader(lightingFragmentShader);
	if (!m_LightingShaderProgram->Link())
	{
		std::cerr << "Failed to link lighting shader program!" << std::endl;
		return;
	}

	// screen quad shader
	vertpath = "resources/shaders/screenQuad.vert";
	fragpath = "resources/shaders/screenQuad.frag";
	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader screenFragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_ScreenShaderProgram = std::make_unique<ShaderProgram>();

	m_ScreenShaderProgram->AttachShader(quadVertexShader);
	m_ScreenShaderProgram->AttachShader(screenFragmentShader);
	if (!m_ScreenShaderProgram->Link())
	{
		std::cerr << "Failed to link post-processing shader program!" << std::endl;
		return;
	}

	// forward pass pbr shader
	vertpath = "resources/shaders/forwardPass.vert";
	fragpath = "resources/shaders/forwardPass.frag";
	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader forwardVertexShader(vertpath, GL_VERTEX_SHADER);
	Shader forwardFragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_ForwardShaderProgram = std::make_unique<ShaderProgram>();

	m_ForwardShaderProgram->AttachShader(forwardVertexShader);
	m_ForwardShaderProgram->AttachShader(forwardFragmentShader);
	if (!m_ForwardShaderProgram->Link())
	{
		std::cerr << "Failed to link forward pass shader program!" << std::endl;
		return;
	}

	// downsample pass pbr shader
	fragpath = "resources/shaders/downsample.frag";
	#ifdef MAGMA_ROOT_DIR
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader downsampleFragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_DownsampleShaderProgram = std::make_unique<ShaderProgram>();

	m_DownsampleShaderProgram->AttachShader(quadVertexShader);
	m_DownsampleShaderProgram->AttachShader(downsampleFragmentShader);
	if (!m_DownsampleShaderProgram->Link())
	{
		std::cerr << "Failed to link forward pass shader program!" << std::endl;
		return;
	}

	// downsample pass pbr shader
	fragpath = "resources/shaders/upsample.frag";
	#ifdef MAGMA_ROOT_DIR
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader upsampleFragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_UpsampleShaderProgram = std::make_unique<ShaderProgram>();

	m_UpsampleShaderProgram->AttachShader(quadVertexShader);
	m_UpsampleShaderProgram->AttachShader(upsampleFragmentShader);
	if (!m_UpsampleShaderProgram->Link())
	{
		std::cerr << "Failed to link forward pass shader program!" << std::endl;
		return;
	}

	// skybox shader
	vertpath = "resources/shaders/skybox.vert";
	fragpath = "resources/shaders/skybox.frag";
	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader skyboxVertexShader(vertpath, GL_VERTEX_SHADER);
	Shader skyboxFragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_SkyboxShaderProgram = std::make_unique<ShaderProgram>();

	m_SkyboxShaderProgram->AttachShader(skyboxVertexShader);
	m_SkyboxShaderProgram->AttachShader(skyboxFragmentShader);
	if (!m_SkyboxShaderProgram->Link())
	{
		std::cerr << "Failed to link forward pass shader program!" << std::endl;
		return;
	}


	SetupShadowMap();
	SetupBloom(w, h);
}

// ---- GAME UPDATE LOGIC ----
void GameLayer::OnUpdate(float dt)
{
	//ZoneScoped;
	float realDT = dt;
	if (dt > 0.1f) dt = 0.1f; // safaty

	double fps = 1.0f / realDT;
	m_TotalFPS += fps;

	double frameTimeMS = realDT * 1000.0;
	m_TotalFrameTime += frameTimeMS;

	m_FrameCount++;
	if (m_FrameCount >= 60)
	{
		m_AvgFPS = m_TotalFPS / m_FrameCount;
		m_AvgFrameTime = m_TotalFrameTime / m_FrameCount;
		m_TotalFPS = 0.0f;
		m_FrameCount = 0;
		m_TotalFrameTime = 0.0f;
	}

	int w, h;
	SDL_GetWindowSize(m_Window, &w, &h);


	// Mouse look
	ImGuiIO& io = ImGui::GetIO();
	if (!m_IsLocalPlayerLoaded && !io.WantCaptureMouse && Magma::Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
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

	// 0 - shadow pass
	if (m_LightProjDirty)
	{
		m_LightProjMatrix = glm::ortho(
			-m_SunShadowOrthoSize,
			m_SunShadowOrthoSize,
			-m_SunShadowOrthoSize,
			m_SunShadowOrthoSize,
			m_SunShadowNearPlane,
			m_SunShadowFarPlane
		);
		m_LightProjDirty = false;
	}

	glm::vec3 playerPos = glm::vec3(0.0f);
	if (m_IsLocalPlayerLoaded)
	{
		auto& transformRef = m_EntityWorld->GetComponent<Craft::TransformComponent>(m_PrimaryCamera);
		playerPos = transformRef.worldMatrix[3];


		m_LightViewMatrix = glm::lookAt(
			(m_SunDirection * -m_SunDistanceMultiplier) + playerPos, // place sun pos somehwere along it's view direction and follow player
			playerPos, // look at player
			glm::vec3(0.0f, 1.0f, 0.0f) // up
		);

		glm::mat4 lightSpaceMatrix = m_LightProjMatrix * m_LightViewMatrix;

		m_ShadowMapShaderProgram->Use(); // use the shadow shader outside render function for the uniforms

		m_ShadowMapShaderProgram->SetUniform("u_LightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowMapFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);

		m_RenderSystem->Render(*m_EntityWorld, *m_ShadowMapShaderProgram, m_WorldStreamer.get(), m_Window, true, false);

		// 1 - geometry pass
		// clear screen completely
		glViewport(0, 0, w, h);
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		m_ShaderProgram->Use();
		m_RenderSystem->Render(*m_EntityWorld, *m_ShaderProgram, m_WorldStreamer.get(), m_Window, false, false);

		// 2 - lighting pass
		glBindFramebuffer(GL_FRAMEBUFFER, m_GLightingPassFBO);

		m_LightingShaderProgram->Use();

		// bind g textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_GPosition->GetID());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_GNormal->GetID());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_GAlbedo->GetID());

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_GMatData->GetID());

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_ShadowMap->GetID());

		m_LightingShaderProgram->SetUniform("u_CameraPosition", glm::vec3(transformRef.worldMatrix[3]));


		//m_LightingShaderProgram->SetUniform("u_CameraPosition", glm::vec3(0.0f));

		m_LightingShaderProgram->SetUniform("u_GPosition", 0);
		m_LightingShaderProgram->SetUniform("u_GNormal", 1);
		m_LightingShaderProgram->SetUniform("u_GAlbedo", 2);
		m_LightingShaderProgram->SetUniform("u_GASME", 3);
		m_LightingShaderProgram->SetUniform("u_ShadowMap", 4);
		m_LightingShaderProgram->SetUniform("u_LightSpaceMatrix", lightSpaceMatrix);
		m_LightingShaderProgram->SetUniform("u_SunColor", m_SunColor);
		m_LightingShaderProgram->SetUniform("u_SunIntensity", (float)m_SunIntensity);
		m_LightingShaderProgram->SetUniform("u_SunDirection", m_SunDirection);
		m_LightingShaderProgram->SetUniform("u_ShadowBiasMin", (float)m_ShadowBiasMin);
		m_LightingShaderProgram->SetUniform("u_ShadowBiasMax", (float)m_ShadowBiasMax);
		m_LightingShaderProgram->SetUniform("u_ShadowFadeDistance", (float)m_ShadowFadeDistance);
		m_LightingShaderProgram->SetUniform("u_AmbientIntensity", (float)m_AmbientIntensity);


		glBindVertexArray(m_ScreenVAO);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Skybox pass
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glCullFace(GL_FRONT);
		m_SkyboxShaderProgram->Use();
		auto& camRef = m_EntityWorld->GetComponent<Craft::CameraComponent>(m_PrimaryCamera);
		glm::mat4 view = glm::mat4(glm::mat3(camRef.viewMatrix));
		glm::mat4 viewProj = camRef.projectionMatrix * view;
		m_SkyboxShaderProgram->SetUniform("u_ViewProjection", viewProj);
		m_SkyboxShaderProgram->SetUniform("u_SunDirection", m_SunDirection);
		m_SkyboxShaderProgram->SetUniform("u_SunIntensity", (float)m_SunIntensity);
		m_SkyboxShaderProgram->SetUniform("u_SunBloomSize", m_SunBloomSize);
		m_SkyboxShaderProgram->SetUniform("u_SunRadius", m_SunRadius);
		m_SkyboxShaderProgram->SetUniform("u_StarSize", m_StarSize);
		m_SkyboxShaderProgram->SetUniform("u_StarDensity", m_StarDensity);

		m_SkyboxShaderProgram->SetUniform("u_DayZenithColor", m_DayZenithColor);
		m_SkyboxShaderProgram->SetUniform("u_DaySunColor", m_DaySunColor);
		m_SkyboxShaderProgram->SetUniform("u_DayHorizonColor", m_DayHorizonColor);
		m_SkyboxShaderProgram->SetUniform("u_SunsetZenithColor", m_SunsetZenithColor);
		m_SkyboxShaderProgram->SetUniform("u_SunsetSunColor", m_SunsetSunColor);
		m_SkyboxShaderProgram->SetUniform("u_SunsetHorizonColor", m_SunsetHorizonColor);
		m_SkyboxShaderProgram->SetUniform("u_NightZenithColor", m_NightZenithColor);
		m_SkyboxShaderProgram->SetUniform("u_NightSunColor", m_NightSunColor);
		m_SkyboxShaderProgram->SetUniform("u_NightHorizonColor", m_NightHorizonColor);

		RenderCube();
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glCullFace(GL_BACK);

		// 2.5 - forward pass
		glBindFramebuffer(GL_FRAMEBUFFER, m_GLightingPassFBO);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		m_ForwardShaderProgram->Use();

		// bind block texture atlasses
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_NetworkManager->GetWorldManager()->m_BlockAtlasTextureAlbedo->GetID());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_NetworkManager->GetWorldManager()->m_BlockAtlasTextureNormal->GetID());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_NetworkManager->GetWorldManager()->m_BlockAtlasTextureASME->GetID());

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_ShadowMap->GetID());

		m_ForwardShaderProgram->SetUniform("u_AlbedoTexture", 0);
		m_ForwardShaderProgram->SetUniform("u_NormalTexture", 1);
		m_ForwardShaderProgram->SetUniform("u_ASMETexture", 2);
		m_ForwardShaderProgram->SetUniform("u_ShadowMap", 3);
		m_ForwardShaderProgram->SetUniform("u_LightSpaceMatrix", lightSpaceMatrix);
		m_ForwardShaderProgram->SetUniform("u_SunColor", m_SunColor);
		m_ForwardShaderProgram->SetUniform("u_SunIntensity", (float)m_SunIntensity);
		m_ForwardShaderProgram->SetUniform("u_SunDirection", m_SunDirection);
		m_ForwardShaderProgram->SetUniform("u_ShadowBiasMin", (float)m_ShadowBiasMin);
		m_ForwardShaderProgram->SetUniform("u_ShadowBiasMax", (float)m_ShadowBiasMax);
		m_ForwardShaderProgram->SetUniform("u_ShadowFadeDistance", (float)m_ShadowFadeDistance);
		m_ForwardShaderProgram->SetUniform("u_AmbientIntensity", (float)m_AmbientIntensity);

		m_RenderSystem->Render(*m_EntityWorld, *m_ForwardShaderProgram, m_WorldStreamer.get(), m_Window, false, true);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		// BLOOM PASS
		RenderBloom(m_GLightingPass->GetID());

		// 3 - post processing pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_ScreenShaderProgram->Use();

		// bind gbuffer textures for reading
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_GLightingPass->GetID());
		m_ScreenShaderProgram->SetUniform("u_ScreenTexture", 0);

		// depth uniforms
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_GDepth->GetID());
		m_ScreenShaderProgram->SetUniform("u_DepthTexture", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_BloomMipTextures[0]->GetID());
		m_ScreenShaderProgram->SetUniform("u_BloomTexture", 2);

		m_ScreenShaderProgram->SetUniform("u_FogNear", m_FogNear); // update to make dependent on render distance
		m_ScreenShaderProgram->SetUniform("u_FogFar", m_FogFar);
		m_ScreenShaderProgram->SetUniform("u_FogDensity", m_FogDensity);
		m_ScreenShaderProgram->SetUniform("u_FogCurve", m_FogCurve);
		m_ScreenShaderProgram->SetUniform("u_Exposure", m_Exposure);
		m_ScreenShaderProgram->SetUniform("u_Saturation", m_Saturation);
		m_ScreenShaderProgram->SetUniform("u_Gamma", m_Gamma);
		m_ScreenShaderProgram->SetUniform("u_BloomStrength", m_BloomStrength);

		m_FogColor = CalcHorizonColors();
		m_ScreenShaderProgram->SetUniform("u_FogColor", m_FogColor);


		glBindVertexArray(m_ScreenVAO);
		glDisable(GL_DEPTH_TEST);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// screenshot 
	if (Magma::Input::IsKeyPressed(SDL_SCANCODE_F2)) { Screenshot(w, h); }


	Magma::Input::Update();
	Magma::AudioEngine::UpdateActiveSounds();

	// ---- NETWORK UPDATE ----
	if (m_NetworkManager->IsRunning())
	{
		m_NetworkManager->Update(dt);
	}
	//FrameMark;
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

	glDeleteFramebuffers(1, &m_GBuffer);
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
	ImGui::Begin("MagmaCraft by Gio Perez Colon");
	if (m_MenuState != MenuState::IN_GAME)
	{
		ImGui::TextUnformatted(logoArt);
		ImGui::Text("by Gio Perez Colon");
	}
	else
	{
		ImGui::Text("---Double click Escape when in game to gain control---");
	}
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Separator();

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
				std::string worldPath;
				if (m_WorldSelector->OnImGuiRender(worldPath))
				{
					auto fullPath = std::filesystem::absolute(worldPath);
					fullPath.make_preferred();
					if (!m_NetworkManager->GetWorldManager().get()->LoadWorld(fullPath.string().c_str()))
					{
						ImGui::Text("Invalid file path.");
						m_NetworkManager->End();
						m_MenuState = MenuState::MAIN_MENU;
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
			ImGui::Text("Press ESCAPE to toggle menu control.");
			ImGui::Text("Press F2 to take screenshot.");
			ImGui::Text("FPS: %.1f", m_AvgFPS);
			ImGui::Text("Frame Time: %.3f", m_AvgFrameTime);
			ImGui::Text("Autosave in %.1f seconds", m_AutoSaveTimer);

			ImGui::BeginDisabled();
			ImGui::Checkbox("VSync", &m_VSyncEnabled); // read only
			ImGui::EndDisabled();

			if (ImGui::Button("Toggle VSync")) { ToggleVSync(); }
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

			if (ImGui::BeginMenu("Lighting Settings"))
			{
				ImGui::DragFloat("Ambient Intensity", &m_AmbientIntensity, 0.01f, 0.0f, 1.0f);

				ImGui::DragFloat("Sun Intensity", &m_SunIntensity, 0.1f, 0.0f, 100.0f);
				ImGui::ColorEdit3("Sun Color", glm::value_ptr(m_SunColor), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
				ImGui::DragFloat("Sun Radius", &m_SunRadius, 0.1f, 0.0f, 100.0f);
				ImGui::DragFloat("Sun Bloom Size", &m_SunBloomSize, 0.1f, 0.0f, 10.0f);

				ImGui::DragFloat("Star Size", &m_StarSize, 0.1f, 0.0f, 5000.0f);
				ImGui::DragFloat("Star Density", &m_StarDensity, 0.01f, 0.0f, 10.0f);

				ImGui::ColorEdit3("Day Zenith Color", glm::value_ptr(m_DayZenithColor), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("Day Horizon Color", glm::value_ptr(m_DayHorizonColor), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("Sunset Zenith Color", glm::value_ptr(m_SunsetZenithColor), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("Sunset Horizon Color", glm::value_ptr(m_SunsetHorizonColor), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("Night Zenith Color", glm::value_ptr(m_NightZenithColor), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
				ImGui::ColorEdit3("Night Horizon Color", glm::value_ptr(m_NightHorizonColor), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);


				// Euler to sundirection

				float pitch = glm::degrees(asin(m_SunDirection.y));
				float yaw = glm::degrees(atan2(-m_SunDirection.x, -m_SunDirection.z));

				bool changed = false;
				ImGui::Text("Sun Direction");
				changed |= ImGui::DragFloat("Sun Pitch", &pitch, 1.0f, -89.0f, 89.0f);
				changed |= ImGui::DragFloat("Sun Yaw", &yaw, 1.0f, -180.0f, 180.0f);
				if (changed)
				{
					float radPitch = glm::radians(pitch);
					float radYaw = glm::radians(yaw);

					glm::vec3 newDir;
					newDir.y = sin(radPitch);
					newDir.x = -sin(radYaw) * cos(radPitch);
					newDir.z = -cos(radYaw) * cos(radPitch);

					m_SunDirection = glm::normalize(newDir);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Post Processing Settings"))
			{
				ImGui::DragFloat("Fog Near", &m_FogNear, 0.1f, 0.0f, 1000.0f);
				ImGui::DragFloat("Fog Far", &m_FogFar, 0.1f, 0.0f, 10000.0f);
				ImGui::DragFloat("Fog Density", &m_FogDensity, 0.001f, 0.0f, 1.0f);
				ImGui::DragFloat("Fog Curve", &m_FogCurve, 0.1f, 0.1f, 256.0f);

				ImGui::DragFloat("Exposure Level", &m_Exposure, 0.1f, 0.0f, 100.0f);
				ImGui::DragFloat("Saturation", &m_Saturation, 0.1f, 0.0f, 10.0f);
				ImGui::DragFloat("Gamma", &m_Gamma, 0.1f, 0.0f, 5.0f);
				ImGui::DragFloat("Bloom Strength", &m_BloomStrength, 0.01f, 0.0f, 5.0f);
				ImGui::DragFloat("Bloom Radius", &m_FilterRadius, 0.001f, 0.0f, 5.0f);
				ImGui::DragFloat("Bloom Weight", &m_BloomWeight, 0.1f, 0.0f, 1.0f);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Shadow Settings"))
			{
				bool changed = false;

				changed |= ImGui::DragFloat("Shadow Near", &m_SunShadowNearPlane, 0.1f, 0.0f, 10000.0f);
				changed |= ImGui::DragFloat("Shadow Far", &m_SunShadowFarPlane, 0.1f, 0.0f, 10000.0f);
				changed |= ImGui::DragFloat("Shadow OrthoSize", &m_SunShadowOrthoSize, 0.1f, 0.0f, 10000.0f);
				ImGui::DragFloat("Shadow Distance", &m_SunDistanceMultiplier, 0.1f, 0.0f, 10000.0f);
				ImGui::DragFloat("Shadow Bias Min", &m_ShadowBiasMin, 0.01f, 0.0f, 1.0);
				ImGui::DragFloat("Shadow Bias Max", &m_ShadowBiasMax, 0.01f, 0.0f, 1.0);
				ImGui::DragFloat("Shadow Fade Distance", &m_ShadowFadeDistance, 0.1f, 0.0f, 10000.0);

				if (changed)
				{
					m_LightProjDirty = true;
				}

				ImGui::EndMenu();
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

	glBindTexture(GL_TEXTURE_2D, m_GLightingPass->GetID());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	// resize gbuffer textures
	glBindTexture(GL_TEXTURE_2D, m_GPosition->GetID());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, m_GNormal->GetID());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, m_GAlbedo->GetID());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, m_GMatData->GetID());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, m_GDepth->GetID());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);


	// Shadow map
	if (m_ShadowMap)
	{
		glBindTexture(GL_TEXTURE_2D, m_ShadowMap->GetID());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
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
	std::cout << "Cleaning up local player..." << std::endl;
	if (m_Player != Craft::NULL_ENTITY && m_EntityWorld->HasEntityID(m_Player))
	{
		m_EntityWorld->RemoveEntity(m_Player);
	}
	m_Player = Craft::NULL_ENTITY;
	m_PrimaryCamera = Craft::NULL_ENTITY;
	m_EntityWorld->SetLocalPlayerID(Craft::NULL_ENTITY);
	m_IsLocalPlayerLoaded = false;
	m_AutoSaveTimer = 0.0f;
	std::cout << "Local player cleaned." << std::endl;
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

void GameLayer::SetupShadowMap()
{
	glGenFramebuffers(1, &m_ShadowMapFBO);

	m_ShadowMap = std::make_unique<Magma::Texture>(
		SHADOW_MAP_RESOLUTION,
		SHADOW_MAP_RESOLUTION,
		GL_TEXTURE_2D,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr
	);
	m_ShadowMap->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_ShadowMap->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_ShadowMap->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	m_ShadowMap->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_ShadowMap->TexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


	glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMap->GetID(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::string vertpath = "resources/shaders/shadowMap.vert";
	std::string fragpath = "resources/shaders/dummy.frag";
	#ifdef MAGMA_ROOT_DIR
		vertpath = std::string(MAGMA_ROOT_DIR) + vertpath;
		fragpath = std::string(MAGMA_ROOT_DIR) + fragpath;
	#endif

	Shader vertexShader(vertpath, GL_VERTEX_SHADER);
	Shader fragmentShader(fragpath, GL_FRAGMENT_SHADER);

	m_ShadowMapShaderProgram = std::make_unique<ShaderProgram>();

	m_ShadowMapShaderProgram->AttachShader(vertexShader);
	m_ShadowMapShaderProgram->AttachShader(fragmentShader);
	if (!m_ShadowMapShaderProgram->Link())
	{
		std::cerr << "Failed to link post-processing shader program!" << std::endl;
		return;
	}
}

void GameLayer::Screenshot(const int& w, const int& h)
{
	// first write the file name with curernt time
	auto currentTime = std::chrono::system_clock::now();
	auto inTimeT = std::chrono::system_clock::to_time_t(currentTime);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&inTimeT), "%Y-%m-%d_%H-%M-%S");
	std::string folderPath = "screenshots/";
	std::filesystem::create_directories(folderPath);
	std::string filename = folderPath + "MagmaCraft_" + ss.str() + ".png";

	std::vector<unsigned char> pixels(w * h * 3);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	stbi_flip_vertically_on_write(true); // opengl 0,0 is bottom left, png is top left

	if (stbi_write_png(filename.c_str(), w, h, 3, pixels.data(), w * 3)) { std::cout << "Screenshot saved! " << filename << std::endl; }
}

// move to window class or something
void GameLayer::ToggleVSync()
{
	if (m_VSyncEnabled)
	{
		SDL_GL_SetSwapInterval(0);
	}
	else
	{
		SDL_GL_SetSwapInterval(1);
	}
	m_VSyncEnabled = !m_VSyncEnabled;
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

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GameLayer::RenderScreenQuad()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(m_ScreenVAO);
	glDisable(GL_DEPTH_TEST);


	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void GameLayer::SetupBloom(const int& w, const int& h)
{
	if (m_BloomFBO != -1)
	{
		glDeleteFramebuffers(1, &m_BloomFBO);
	}

	glGenFramebuffers(1, &m_BloomFBO);

	m_BloomMapTextureSizes.clear();
	m_BloomMipTextures.clear();
	m_BloomMipWeights.clear();

	m_BloomMapTextureSizes.resize(m_BloomMipCount);
	m_BloomMipTextures.resize(m_BloomMipCount);
	m_BloomMipWeights.resize(m_BloomMipCount);

	int w_ = w;
	int h_ = h;

	float initialWeight = 1.0f;
	for (int i = 0; i < m_BloomMipCount; i++)
	{
		m_BloomMapTextureSizes[i] = glm::ivec2(w_, h_);
		m_BloomMipWeights[i] = initialWeight;
		m_BloomMipTextures[i] = std::make_unique<Magma::Texture>(
			std::max(1, w_),
			std::max(1, h_),
			GL_TEXTURE_2D,
			GL_RGBA16F,
			GL_RGBA,
			GL_FLOAT,
			nullptr
		);
		m_BloomMipTextures[i]->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		m_BloomMipTextures[i]->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		m_BloomMipTextures[i]->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_BloomMipTextures[i]->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		w_ = std::max(1, w_ >> 1);
		h_ = std::max(1, h_ >> 1);

		glBindTexture(GL_TEXTURE_2D, 0);
		initialWeight *= m_BloomWeight;
	}
}

void GameLayer::RenderBloom(unsigned int texID)
{
	bool karisAvg = true;
	glBindFramebuffer(GL_FRAMEBUFFER, m_BloomFBO);

	m_DownsampleShaderProgram->Use();
	if (karisAvg)
	{
		m_DownsampleShaderProgram->SetUniform("u_MipLevel", 0);
	}

	// for each mip level
	unsigned int curTexID = texID;
	for (int level = 0; level < m_BloomMipCount; level++)
	{
		const unsigned int w = m_BloomMapTextureSizes[level].x;
		const unsigned int h = m_BloomMapTextureSizes[level].y;

		// downsample the current mip level
		glViewport(0, 0, w, h);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BloomMipTextures[level]->GetID(), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, curTexID);
		m_DownsampleShaderProgram->SetUniform("u_Input", 0);
		m_DownsampleShaderProgram->SetUniform("u_Gamma", m_Gamma);
		m_DownsampleShaderProgram->SetUniform("u_TexelSize", glm::vec2(1.0 / w, 1.0 / h));

		RenderScreenQuad();

		curTexID = m_BloomMipTextures[level]->GetID();

		if (level == 0)
		{
			m_DownsampleShaderProgram->SetUniform("u_MipLevel", 1);
		}
	}

	// upsample
	m_UpsampleShaderProgram->Use();

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	for (int level = m_BloomMipCount - 1; level > 0; level--)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_BloomMipTextures[level]->GetID());

		m_UpsampleShaderProgram->SetUniform("u_Input", 0);
		m_UpsampleShaderProgram->SetUniform("u_FilterRadius", m_FilterRadius);
		glViewport(0, 0, m_BloomMapTextureSizes[level - 1].x, m_BloomMapTextureSizes[level - 1].y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BloomMipTextures[level - 1]->GetID(), 0);

		RenderScreenQuad();
	}

	glDisable(GL_BLEND);
}

glm::vec3 GameLayer::CalcHorizonColors()
{
	float sunHeight = -m_SunDirection.y;

	float sunsetMix = glm::smoothstep(-0.1f, 0.2f, sunHeight) * (1.0f - glm::smoothstep(0.2f, 0.5f, sunHeight));
	float dayMix = glm::smoothstep(0.2f, 0.4f, sunHeight);
	float nightMix = 1.0f - glm::smoothstep(-0.2f, 0.1f, sunHeight);

	glm::vec3 finalColor = m_DayHorizonColor * dayMix + m_SunsetHorizonColor * sunsetMix + m_NightHorizonColor * nightMix;

	return glm::clamp(finalColor, 0.0f, 1.0f);
}

void GameLayer::CreateCube()
{
	glGenVertexArrays(1, &m_CubeVAO);
	glGenBuffers(1, &m_CubeVBO);
	glGenBuffers(1, &m_CubeEBO);

	glBindVertexArray(m_CubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Magma::cubeVertices), Magma::cubeVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Magma::cubeIndices), Magma::cubeIndices, GL_STATIC_DRAW);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GameLayer::RenderCube()
{
	glBindVertexArray(m_CubeVAO);
	GLsizei indexCount = sizeof(Magma::cubeIndices) / sizeof(Magma::cubeIndices[0]);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}