#include "Application.h"
#include "utils/Utils.h"
#include "core/gb/cpu/CPU.h"
#include "core/gb/AddressBus.h"
#include "core/gb/RAM.h"
#include "core/gb/MemoryController.h"
#include "core/gb/ROM.h"
#include "utils/FileManager.h"
#include "core/gb/Timer.h"


#include "glm/vec4.hpp"
#include "imgui.h"
#include "external/imgui_impl_glfw.h"
#include "external/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <iomanip>
#include <memory>

#define BIND_READ(x, func) std::bind(func, &x, std::placeholders::_1)
#define BIND_WRITE(x, func) std::bind(func, &x, std::placeholders::_1, std::placeholders::_2)

namespace gbemu {






	Application::Application() :
		m_Window(nullptr), m_TestMenu(), m_RAM(nullptr), m_ROM(nullptr), m_Bus(), m_CPU(nullptr), m_IsRunning(true), m_StepMode(false), m_Execute(false)
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

		glfwInit();

		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		m_Window = glfwCreateWindow(400, 400, "GameboyEmulator", nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);

		glfwSetWindowUserPointer(m_Window, this);
		glfwSetWindowCloseCallback(m_Window, onWinowClosed);

		glfwSwapInterval(0);

		int result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		gladLoadGL();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		Timer timer{};


		m_RAM = std::make_unique<RAM>(0x8000, 0xFFFF);
		m_ROM = std::make_unique<ROM>(FileManager::readFile("../TestRoms/blargg/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb"));

		m_Bus.connect(MemoryController(0x0000, 0x7FFF, BIND_READ(*m_ROM, &ROM::read), BIND_WRITE(*m_ROM, &ROM::write)));
		m_Bus.connect(MemoryController(0x8000, 0xFFFF, BIND_READ(*m_RAM, &RAM::read), BIND_WRITE(*m_RAM, &RAM::write)));

		m_CPU = std::make_unique<SharpSM83>(m_Bus);
	}

	void Application::update()
	{
		constexpr uint32_t i = sizeof(SharpSM83);

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
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		m_TestMenu.updateGUI();


		ImGui::Begin("Cpu registers");
		
		ImGui::Text(m_CPU->registersOut().c_str());

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGuiIO io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		ImGui::GetDrawData();

		glfwSwapBuffers(m_Window);
	}

	void Application::pollEvents()
	{
		glfwPollEvents();
	}

	void Application::cleanup()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
}


