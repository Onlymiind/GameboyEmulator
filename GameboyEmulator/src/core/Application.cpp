#include "Application.h"
#include "utils/Utils.h"
#include "core/gb/cpu/CPU.h"
#include "core/gb/AddressBus.h"
#include "core/gb/RAM.h"
#include "core/gb/MemoryController.h"
#include "core/gb/ROM.h"
#include "utils/FileManager.h"
#include "core/gb/Timer.h"
#include "core/gb/cpu/Operation.h"
#include "core/gb/cpu/Decoder.h"

#include <SFML/Window/Keyboard.hpp>

#include <iostream>
#include <iomanip>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <exception>
#include <future>
#include <chrono>

#define BIND_READ(x, func) std::bind(func, &x, std::placeholders::_1)
#define BIND_WRITE(x, func) std::bind(func, &x, std::placeholders::_1, std::placeholders::_2)

namespace gb {

    void Application::draw()
    {

    }

    Application::Application() :
        m_RAM(computeSizeFromAddresses(0x8000, 0xFEFF)), m_ROM(), m_Leftover(computeSizeFromAddresses(0xFF80, 0xFFFF)), m_GBIO(),
        m_Bus(), m_CPU(m_Bus, m_InterruptEnable, m_InterruptFlags, m_Decoder), m_IsRunning(true), m_EmulatorRunning(false), m_JustStarted(false)
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
        m_Bus.connect(MemoryController(0x0000, 0x7FFF, m_ROM));
        m_Bus.connect(MemoryController(0x8000, 0xFEFF, m_RAM));
        m_Bus.connect(MemoryController(0xFF80, 0xFFFF, m_Leftover));
        m_Bus.connect(MemoryController(0xFF00, 0xFF7F, m_GBIO));

        std::cout << R"(
****************************
*  Welcome to GB emulator  *
****************************
            )" << "\n";
    }

    void Application::update()
    {
        static auto result = std::async(std::launch::async, sf::Keyboard::isKeyPressed, sf::Keyboard::Escape);


        do
        {
            try
            {
                m_CPU.tick();
            }
            catch(const std::exception& e)
            {
                m_EmulatorRunning = false;
                std::cerr << e.what() << '\n';
                break;
            }
            
        } while (!m_CPU.isFinished());
        
        auto status = result.wait_for(std::chrono::microseconds(0));
        if (status == std::future_status::ready)
        {
            if(!m_JustStarted && result.get())
            {
                m_EmulatorRunning = false;
            }
            result = std::async(std::launch::async, sf::Keyboard::isKeyPressed, sf::Keyboard::Escape);
            if(m_JustStarted) m_JustStarted = false;
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
                std::cout << "Selected directory doesn't exist\n";
            }

            break;
        case CommandType::Help:
            std::cout << "-help - show this text\n"
                << "-romdir <path> -set ROM directory\n"
                << "-run <name> -run a ROM\n"
                << "-ls - list all ROMs in current directory\n"
                << "-config - show current ROM directory\n"
                << "-quit - quit the emulator\n"
                << "During execution:\n"
                << "ESC - stop execution\n";
            break;
        case CommandType::SetRomDir:
            m_TestPath = cmd.Argument;
            if (m_TestPath.back() != '/' || m_TestPath.back() != '\\')
            {
                m_TestPath.push_back('/');
            }
            break;
        case CommandType::Quit:
            m_IsRunning = false;
            break;
        case CommandType::RunRom:

            if (std::filesystem::exists(m_TestPath + cmd.Argument + m_Extension))
            {
                m_ROM.setData(FileManager::readFile(m_TestPath + cmd.Argument + m_Extension));
                m_CPU.reset();
                m_EmulatorRunning = true;
                m_JustStarted = true;
            }
            else
            {
                std::cout << "ROM doesn't exist in the specified directory\n";
            }

            break;
        case CommandType::Config:
            std::cout << "Current ROM directory: " << std::filesystem::absolute(m_TestPath) << "\n";
            break;
        case CommandType::Invalid:
            std::cout << "Invalid command. Use -help for command list\n";
        }
    }

    void Application::cleanup()
    {

    }


    std::istream& operator>>(std::istream& is, Command& command)
    {
        std::string cmd;
        std::getline(is, cmd);


        command = Parser().parse(cmd);

        return is;
    }

    Command Parser::parse(const std::string& text) const
    {
        Command result{ CommandType::None, {} };

        for (const CommandInfo& info : m_Commands)
        {
            std::string::size_type pos = text.find(info.Name);
            if (pos != std::string::npos)
            {
                result.Type = info.Type;

                result.Argument = getArguments(text, info, pos);

                if (info.HasArguments && result.Argument.empty())
                {
                    result.Type = CommandType::Invalid;
                }

                break;
            }
        }

        if (result.Type == CommandType::None && !text.empty())
        {
            result.Type = CommandType::Invalid;
        }

        return result;
    }

    std::string Parser::getArguments(const std::string& text, const Parser::CommandInfo& info, size_t cmdBegin) const
    {
        if (info.HasArguments)
        {
            auto argIt = std::find_if(text.begin() + cmdBegin + info.Name.length(), text.end(),
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


