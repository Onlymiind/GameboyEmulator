#include "Application.h"
#include "utils/Utils.h"
#include "core/gb/CPU.h"


#include "glm/vec4.hpp"
#include "SFML/Graphics.hpp"

#include <iostream>
#include <iomanip>


namespace gbemu {


	Application::Application() :
		m_Window({ 400, 400 }, "GameboyEmulator"), m_IsRunning(true) 
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
		SharpSM83 cpu;

		std::cout << cpu.registersOut() << "\n";
	}

	void Application::update()
	{
		sf::CircleShape circle(40);
		circle.setFillColor(sf::Color::Red);

		m_Window.clear(colorToSFML({ 0.25, 0.1, 0.25, 1.0 }));
		m_Window.draw(circle);
		m_Window.display();

	}

	void Application::pollEvents()
	{
		sf::Event event{};
		while (m_Window.pollEvent(event)) if (event.type == sf::Event::Closed) m_IsRunning = false;
	}

	void Application::cleanup()
	{
		m_Window.close();
	}
}


