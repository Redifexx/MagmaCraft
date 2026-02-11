#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "imgui.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>

#include <glm/glm.hpp>
#include <iostream>
#include <filesystem>

#include "Core/Window.h"
#include "Core/LayerStack.h"
#include "Core/ImGuiLayer.h"
#include "Core/GameLayer.h"
#include "Core/Camera.h"
#include "Core/Input.h"
#include <Core/AudioEngine.h>

int main(int argc, char* argv[])
{
    // Window Creation
	Magma::Window window;
	if (!window.Init(1280, 720, "Magma Framework")) { return -1; }
	window.SetVSync(0);

    // Layers
	Magma::LayerStack layerStack;

	Magma::ImGuiLayer* imGuiLayer = new Magma::ImGuiLayer(window.GetSDLWindow(), window.GetGLContext());
	layerStack.PushLayer(imGuiLayer);

	Magma::GameLayer* gameLayer = new Magma::GameLayer(window.GetSDLWindow());
	layerStack.PushLayer(gameLayer);

	window.SetResizeCallback([&](int width, int height)
	{
		gameLayer->OnResize(width, height);
	});

	gameLayer->OnResize(1280, 720);

    // Timing
	Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
	Uint64 lastCounter = SDL_GetPerformanceCounter();

	float targetFrameTime = 1000.0f / 240.0f;

	// Input
	Magma::Input::Init();

	// Audio
	Magma::AudioEngine::Init();

	SDL_GL_SetSwapInterval(1);

    // Main Loop
    bool isRunning = true;
    while (isRunning)
	{
		// Timing cont.
		Uint64 currentCounter = SDL_GetPerformanceCounter();
		Uint64 counterElapsed = currentCounter - lastCounter;

		float deltaTime = (float)counterElapsed / (float)performanceFrequency;

		lastCounter = currentCounter;

		// Window Handling
		window.PollEvents(isRunning);

		if (Magma::Input::IsKeyPressed(SDL_SCANCODE_ESCAPE))
		{
			SDL_SetWindowRelativeMouseMode(window.GetSDLWindow(), false);
		}

        // Game Logic
        for (Magma::Layer* layer : layerStack)
        {
			layer->OnUpdate(deltaTime);
        }

		// ImGui Rendering
		imGuiLayer->Begin();
        for (Magma::Layer* layer : layerStack)
        {
			layer->OnImGuiRender(deltaTime);
        }
		imGuiLayer->End();

        window.SwapBuffers();
    }

    window.Shutdown();

    return 0;
}