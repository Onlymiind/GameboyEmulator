#pragma once
#include "core/gb/RAM.h"
#include "core/gb/ROM.h"
#include "core/gb/AddressBus.h"
#include "core/gb/cpu/CPU.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <memory>


namespace gbemu {



	class Application {
	public:
		Application();
		~Application();


		void run();

	private:

		void init();
		void update();
		void pollEvents();
		void cleanup();

	private:

		SDL_Window*   m_Window;
		SDL_Surface*  m_Surface;
		SDL_Renderer* m_Renderer;

		std::unique_ptr<RAM> m_RAM;
		std::unique_ptr<ROM> m_ROM;
		AddressBus m_Bus;
		std::unique_ptr<SharpSM83> m_CPU;

		const uint8_t m_InstructionPerFrame{ 100 };
		bool m_IsRunning;
		bool m_StepMode;
		bool m_Execute;
	};
}