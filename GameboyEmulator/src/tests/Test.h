//#pragma once
//#include <SFML/Graphics.hpp>
//
//
//#include <memory>
//#include <string_view>
//#include <vector>
//#include <utility>
//#include <functional>
//
//namespace gbemu {
//
//
//	class Test {
//	public:
//		Test() {}
//		~Test() = default;
//
//		virtual void update()    = 0;
//		virtual void updateGUI() = 0;
//		virtual void render()    = 0;
//	};
//
//
//	class Menu {
//	public:
//		Menu() :
//			p_Test(nullptr) 
//		{}
//		~Menu() = default;
//
//		void update();
//		void updateGUI();
//		void render(sf::RenderWindow& window);
//	private:
//		std::unique_ptr<Test> p_Test;
//
//		const std::vector<std::pair<std::string_view, std::function<Test* ()>>> m_TestBase =
//		{ {
//
//		} };
//	};
//}