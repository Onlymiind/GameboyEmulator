#pragma once
#include "core/gb/RAM.h"
#include "core/gb/ROM.h"
#include "core/gb/AddressBus.h"
#include "core/gb/cpu/CPU.h"
#include "tests/Test.h"

#include <GLFW/glfw3.h>

#include <memory>
#include <filesystem>
#include <iostream>


namespace gb {



	class Application {
	public:
		Application();
		~Application();


		void run();
		void reset();
		void finishedCallback() { m_EmulatorRunning = false; std::cout << "Done\n"; }

	private:

		void init();
		void update();
		void pollEvents();
		void cleanup();

		static inline void onWinowClosed(GLFWwindow* window) 
		{
			Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
			app->m_IsRunning = false;
		}

	private:

		GLFWwindow* m_Window;

		Menu m_TestMenu;


		std::unique_ptr<RAM> m_RAM;
		std::unique_ptr<ROM> m_ROM;
		AddressBus m_Bus;
		std::unique_ptr<SharpSM83> m_CPU;

		const uint8_t m_InstructionPerFrame{ 100 };
		bool m_IsRunning;
		bool m_EmulatorRunning;
		bool m_StepMode;
		bool m_Execute;


		std::filesystem::path m_TestPath{ "../TestRoms/blargg/cpu_instrs/individual/" };
	};
}