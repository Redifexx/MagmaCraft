#include "Core/Window.h"

using namespace Magma;

bool Window::Init(int width, int height, const std::string& title)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << "SDL failed to init: " << SDL_GetError() << std::endl;
		return 1;
	}

	// OpenGL 4.6
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Window Creation
	m_Window = SDL_CreateWindow(
		title.c_str(),
		width,
		height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (!m_Window)
	{
		std::cerr << "Window failed: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return false;
	}

	// OpenGL Context Creation
	m_GLContext = SDL_GL_CreateContext(m_Window);
	if (!m_GLContext)
	{
		std::cerr << "OpenGL context failed: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
		return false;
	}

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cerr << "Failed to initialize Glad" << std::endl;
		SDL_GL_DestroyContext(m_GLContext);
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
		return false;
	}

	return true;
}

void Window::SwapBuffers()
{
	SDL_GL_SwapWindow(m_Window);
}

// SDL callbacks are handled here
void Window::PollEvents(bool& isRunning)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_WINDOW_RESIZED)
		{
			int width = event.window.data1;
			int height = event.window.data2;

			glViewport(0, 0, width, height);

			if (m_ResizeCallback)
			{
				m_ResizeCallback(width, height);
			}
		}

		if (event.type == SDL_EVENT_QUIT)
			isRunning = false;

		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(m_Window))
			isRunning = false;
	}
}

void Window::Shutdown()
{
	if (m_GLContext)
	{
		SDL_GL_DestroyContext(m_GLContext);
		m_GLContext = nullptr;
	}
	if (m_Window)
	{
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
	}
	SDL_Quit();
}

void Window::SetVSync(int vSync)
{
	if (!SDL_GL_SetSwapInterval(vSync))
	{
		std::cerr << "Failed to set VSync: " << SDL_GetError() << std::endl;
	}
}

void Window::GetWindowSize(int& width, int& height) const
{
	SDL_GetWindowSize(m_Window, &width, &height);
}