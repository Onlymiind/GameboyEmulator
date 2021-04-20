#include "Application.h"
#include "utils/Utils.h"
#include "core/gb/CPU.h"
#include "core/gb/AddressBus.h"
#include "core/gb/RAM.h"
#include "core/gb/MemoryController.h"
#include "core/gb/ROM.h"
#include "utils/FileManager.h"


#include "glm/vec4.hpp"
#include "SFML/Graphics.hpp"

#include <iostream>
#include <iomanip>
#include <memory>


#define BIND_READ(x, func) std::bind(func, &x, std::placeholders::_1)
#define BIND_WRITE(x, func) std::bind(func, &x, std::placeholders::_1, std::placeholders::_2)

namespace gbemu {


	Application::Application() :
		m_Window({ 400, 400 }, "GameboyEmulator"), m_RAM(nullptr), m_ROM(nullptr), m_Bus(), m_CPU(nullptr), m_IsRunning(true), m_StepMode(false), m_Execute(false)
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
		m_RAM = std::make_unique<RAM>(0x8000, 0xFFFF);
		m_ROM = std::make_unique<ROM>(FileManager::readFile("../TestRoms/blargg/cpu_instrs/individual/02-interrupts.gb"));

		m_Bus.connect(MemoryController(0x0000, 0x7FFF, BIND_READ(*m_ROM, &ROM::read), BIND_WRITE(*m_ROM, &ROM::write)));
		m_Bus.connect(MemoryController(0x8000, 0xFFFF, BIND_READ(*m_RAM, &RAM::read), BIND_WRITE(*m_RAM, &RAM::write)));

		m_CPU = std::make_unique<SharpSM83>(m_Bus);
	}

	void Application::update()
	{
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
		else m_CPU->tick();

		sf::CircleShape circle(40);
		circle.setFillColor(sf::Color::Red);

		m_Window.clear(colorToSFML({ 0.25, 0.1, 0.25, 1.0 }));
		m_Window.draw(circle);
		m_Window.display();

	}

	void Application::pollEvents()
	{
		sf::Event event{};
		while (m_Window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) m_IsRunning = false;

			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) m_StepMode = !m_StepMode;
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::A)
			{
				m_Execute = true;
			}
		}
	}

	void Application::cleanup()
	{
		std::cout << m_CPU->registersOut() << "\n";
		m_Window.close();
	}
}


