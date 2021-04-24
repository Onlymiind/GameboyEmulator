#include "Application.h"
#include "utils/Utils.h"
#include "core/gb/cpu/CPU.h"
#include "core/gb/AddressBus.h"
#include "core/gb/RAM.h"
#include "core/gb/MemoryController.h"
#include "core/gb/ROM.h"
#include "utils/FileManager.h"


#include "glm/vec4.hpp"
#include "imgui.h"
#include <glad/glad.h>
#include "external/imgui_impl_sdl.h"
#include "external/imgui_impl_opengl3.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <iomanip>
#include <memory>

#define BIND_READ(x, func) std::bind(func, &x, std::placeholders::_1)
#define BIND_WRITE(x, func) std::bind(func, &x, std::placeholders::_1, std::placeholders::_2)

namespace gbemu {


	Application::Application() :
		m_Window(nullptr), m_RAM(nullptr), m_ROM(nullptr), m_Bus(), m_CPU(nullptr), m_IsRunning(true), m_StepMode(false), m_Execute(false)
	{
		init();
	}

	Application::~Application()
	{
		cleanup();
	}

	void Application::run()
	{
		while (m_IsRunning) {
			update();
			pollEvents();
		}
	}

	void Application::init()
	{

		SDL_Init(SDL_INIT_EVERYTHING);
		m_Window = SDL_CreateWindow("GameboyEmulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 400, SDL_WINDOW_OPENGL);
		m_Surface = SDL_GetWindowSurface(m_Window);
		m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
		SDL_GLContext context = SDL_GL_CreateContext(m_Window);
		SDL_GL_MakeCurrent(m_Window, context);


		int result = gladLoadGLLoader(static_cast<GLADloadproc>(SDL_GL_GetProcAddress));
		gladLoadGL();
		

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplSDL2_InitForOpenGL(m_Window, SDL_GL_GetCurrentContext());
		ImGui_ImplOpenGL3_Init("#version 460");


		m_RAM = std::make_unique<RAM>(0x8000, 0xFFFF);
		m_ROM = std::make_unique<ROM>(FileManager::readFile("../TestRoms/blargg/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb"));

		m_Bus.connect(MemoryController(0x0000, 0x7FFF, BIND_READ(*m_ROM, &ROM::read), BIND_WRITE(*m_ROM, &ROM::write)));
		m_Bus.connect(MemoryController(0x8000, 0xFFFF, BIND_READ(*m_RAM, &RAM::read), BIND_WRITE(*m_RAM, &RAM::write)));

		m_CPU = std::make_unique<SharpSM83>(m_Bus);
	}

	void Application::update()
	{
		//static sf::Clock clock{};
		bool demo{ true };








		if (m_StepMode) {
			if (m_Execute) 
			{
				if (m_CPU->isFinished()) m_CPU->tick();
				while (!m_CPU->isFinished()) 
				{
					m_CPU->tick();
				}
				m_Execute = false;
			}
		}
		else
		{
			for (uint8_t i{ 0 }; i < m_InstructionPerFrame; ++i)
			{
				if (m_CPU->isFinished()) m_CPU->tick();
				while (!m_CPU->isFinished()) m_CPU->tick();
			}
		}


		glm::vec4 clearColor{ 0.25f, 0.10f, 0.25f, 1.0f };
		glm::u8vec4 unnormColor = unnormalizeColor(clearColor);

		SDL_Color color;

		int res = SDL_SetRenderDrawColor(m_Renderer, unnormColor.r, unnormColor.g, unnormColor.b, unnormColor.a);
		int result = SDL_RenderClear(m_Renderer);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(m_Window);
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		ImGuiIO io = ImGui::GetIO();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			//SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

		SDL_RenderPresent(m_Renderer);

	}

	void Application::pollEvents()
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event)) {

			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) m_IsRunning = false;

		}
	}

	void Application::cleanup()
	{
		std::cout << m_CPU->registersOut() << "\n";
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
	}
}


