#include "Application.h"
#include "utils/Utils.h"
#include "core/gb/cpu/CPU.h"
#include "core/gb/AddressBus.h"
#include "core/gb/RAM.h"
#include "core/gb/MemoryController.h"
#include "core/gb/ROM.h"
#include "utils/FileManager.h"
#include "core/gb/Timer.h"

#include <SFML/Window.hpp>

#include <iostream>
#include <iomanip>
#include <memory>
#include <filesystem>
#include <algorithm>

#define BIND_READ(x, func) std::bind(func, &x, std::placeholders::_1)
#define BIND_WRITE(x, func) std::bind(func, &x, std::placeholders::_1, std::placeholders::_2)

namespace gb {


	Application::Application() :
		m_Window(nullptr), m_TestMenu(), m_RAM(nullptr), m_ROM(nullptr), m_Bus(), m_CPU(nullptr), m_IsRunning(true), m_StepMode(false), m_Execute(false),
		m_EmulatorRunning(false)
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
			if (m_EmulatorRunning)
			{
				update();
			}
			else
			{
				pollCommands();
			}
		}
	}

	void Application::init()
	{
		m_RAM = std::make_unique<RAM>(0x8000, 0xFFFF);
		m_ROM = std::make_unique<ROM>();

		m_Bus.connect(MemoryController(0x0000, 0x7FFF, BIND_READ(*m_ROM, &ROM::read), BIND_WRITE(*m_ROM, &ROM::write)));
		m_Bus.connect(MemoryController(0x8000, 0xFFFF, BIND_READ(*m_RAM, &RAM::read), BIND_WRITE(*m_RAM, &RAM::write)));

		m_CPU = std::make_unique<SharpSM83>(m_Bus);


		std::cout << R"(
****************************
*  Welcome to GB emulator  *
****************************
			)" << "\n";
	}

	void Application::update()
	{
		m_CPU->tick();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		{
			m_EmulatorRunning = false;
		}
	}

	void Application::pollCommands()
	{
		std::cout << ">";

		Command cmd;

		std::cin >> cmd;

		switch (cmd.Type)
		{
		case CommandType::List:

			if (std::filesystem::exists(m_TestPath))
			{
				for (const auto& file : std::filesystem::directory_iterator(m_TestPath))
				{
					if (file.path().extension().string() == ".gb")
					{
						std::cout << "ROM: " << file.path().filename() << "\n";
					}
				}
			}
			else
			{
				std::cout << "Invalid rom directory\n";
			}

			break;
		case CommandType::Help:
			std::cout << "-help - show this text\n"
				      << "-romdir <path> -set ROM directory\n"
				      << "-run <name> -run a ROM\n"
				      << "-list - list all ROMs in current directory\n"
				      << "-config - show current ROM directory\n"
					  << "-quit - quit the emulator\n";
			break;
		case CommandType::SetRomDir:
			m_TestPath = cmd.Argument;
			break;
		case CommandType::Quit:
			m_IsRunning = false;
			break;
		case CommandType::RunRom:

			if (std::filesystem::exists(m_TestPath + cmd.Argument + m_Extension))
			{
				m_ROM->setData(FileManager::readFile(m_TestPath + cmd.Argument + m_Extension));
				m_CPU->reset();
				m_EmulatorRunning = true;
			}
			else
			{
				std::cout << "ROM doesn't exist in the specified directory\n";
			}

			break;
		case CommandType::Config:
			std::cout << "Current ROM directory: " << std::filesystem::absolute(m_TestPath) << "\n";
			break;
		default:
			std::cout << "Invalid command\n";
		}
	}

	void Application::cleanup()
	{
	}


	std::istream& operator>>(std::istream& is, Command& command)
	{
		std::string cmd;
		std::getline(is, cmd);

		std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](const char c) { return std::tolower(c); });

		command = Parser().parse(cmd);

		return is;
	}

	Command Parser::parse(const std::string& text) const
	{
		Command result{ CommandType::None, {} };

		for (const CommandInfo& info : m_Commands)
		{
			if (text.find(info.Name) != std::string::npos)
			{
				result.Type = info.Type;

				result.Argument = getArguments(text, info);

				if (info.HasArguments && result.Argument.empty())
				{
					result.Type = CommandType::None;
				}

				break;
			}
		}

		return result;
	}

	std::string Parser::getArguments(const std::string& text, const Parser::CommandInfo& info) const
	{
		if (info.HasArguments)
		{
			auto argIt = std::find_if(text.begin() + info.Name.length(), text.end(),
				[](const char c) { return c != ' '; });
			if (argIt != text.end())
			{
				return std::string(argIt, text.end());
			}
			else
			{
				return {};
			}
		}

		return {};
	}
}


