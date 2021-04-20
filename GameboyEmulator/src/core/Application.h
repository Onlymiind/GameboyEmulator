#pragma once
#include "gb/RAM.h"
#include "gb/ROM.h"
#include "gb/AddressBus.h"
#include "gb/CPU.h"

#include <SFML/Graphics.hpp>

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

		sf::RenderWindow m_Window;

		std::unique_ptr<RAM> m_RAM;
		std::unique_ptr<ROM> m_ROM;
		AddressBus m_Bus;
		std::unique_ptr<SharpSM83> m_CPU;


		bool m_IsRunning;
		bool m_StepMode;
		bool m_Execute;
	};
}