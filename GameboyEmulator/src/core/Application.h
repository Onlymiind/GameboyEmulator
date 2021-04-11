#pragma once
#include <SFML/Graphics.hpp>




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

		bool m_IsRunning;
	};
}