#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "imgui.h"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>

#include <glm/glm.hpp>
#include <iostream>
#include <filesystem>

#include "Window.h"
#include "LayerStack.h"
#include "ImGuiLayer.h"
#include "GameLayer.h"
#include "Camera.h"
#include "Input.h"
#include <AudioEngine.h>

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

	Magma::GameLayer* gameLayer = new Magma::GameLayer();
	layerStack.PushLayer(gameLayer);

	window.SetResizeCallback([&](int width, int height)
	{
		gameLayer->OnResize(width, height);
	});

	gameLayer->OnResize(1280, 720);
	gameLayer->SetWindow(window.GetSDLWindow());

    // Timing
	Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
	Uint64 lastCounter = SDL_GetPerformanceCounter();

	float targetFrameTime = 1000.0f / 240.0f;

	// Input
	Magma::Input::Init();

	// Audio
	Magma::AudioEngine::Init();

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

        int w_, h_;
		window.GetWindowSize(w_, h_);

        glViewport(0, 0, w_, h_);
        glClearColor(0.143f, 0.265f, 0.310f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
			layer->OnImGuiRender();
        }
		imGuiLayer->End();

        window.SwapBuffers();
    }

    window.Shutdown();

    return 0;
}