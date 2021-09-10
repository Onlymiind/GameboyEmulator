#include "Application.h"
#include "utils/Utils.h"
#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "gb/RAM.h"
#include "gb/MemoryController.h"
#include "gb/ROM.h"
#include "utils/FileManager.h"
#include "gb/Timer.h"
#include "gb/cpu/Operation.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <exception>
#include <future>
#include <chrono>

#define BIND_READ(x, func) std::bind(func, &x, std::placeholders::_1)
#define BIND_WRITE(x, func) std::bind(func, &x, std::placeholders::_1, std::placeholders::_2)

namespace emulator
{
    void Application::draw()
    {

    }

    Application::Application() :
        RAM_(computeSizeFromAddresses(0x8000, 0xFEFF)), ROM_(), leftover_(computeSizeFromAddresses(0xFF80, 0xFFFF)), GBIO_(),
        bus_(), CPU_(bus_, interrupt_enable_, interrupt_flags_, decoder_), is_running_(true), emulator_running_(false), just_started_(false)
    {
        init();
    }

    Application::~Application()
    {
        cleanup();
    }

    void Application::run()
    {
        while (is_running_) {

            if (emulator_running_)
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
        bus_.connect(gb::MemoryController(0x0000, 0x7FFF, ROM_));
        bus_.connect(gb::MemoryController(0x8000, 0xFEFF, RAM_));
        bus_.connect(gb::MemoryController(0xFF80, 0xFFFF, leftover_));
        bus_.connect(gb::MemoryController(0xFF00, 0xFF7F, GBIO_));

        std::cout << R"(
****************************
*  Welcome to GB emulator  *
****************************
            )" << "\n";
    }

    void Application::update()
    {
        //For infinite loop detection
        static uint16_t oldPC = CPU_.getProgramCounter();
        do
        {
            try
            {
                CPU_.tick();
            }
            catch(const std::exception& e)
            {
                emulator_running_ = false;
                std::cerr << e.what() << '\n';
                break;
            }
            
        } while (!CPU_.isFinished());

        if(oldPC == CPU_.getProgramCounter())
        {
            emulator_running_ = false;
        }

        oldPC = CPU_.getProgramCounter();
    }

    void Application::pollCommands()
    {
        std::cout << ">";
        std::string cmd_str;
        std::getline(std::cin, cmd_str);

        Command cmd = command_parser_.parse(cmd_str);

        switch (cmd.type)
        {
        case CommandType::List:

            if (std::filesystem::exists(test_path_))
            {
                for (const auto& file : std::filesystem::directory_iterator(test_path_))
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
            test_path_ = cmd.argument;
            if (test_path_.back() != '/' || test_path_.back() != '\\')
            {
                test_path_.push_back('\\');
            }
            break;
        case CommandType::Quit:
            is_running_ = false;
            break;
        case CommandType::RunRom:

            if (std::filesystem::exists(test_path_ + cmd.argument + extension_))
            {
                ROM_.setData(FileManager::readFile(test_path_ + cmd.argument + extension_));
                CPU_.reset();
                emulator_running_ = true;
                just_started_ = true;
            }
            else
            {
                std::cout << "ROM doesn't exist in the specified directory\n";
            }

            break;
        case CommandType::Config:
            std::cout << "Current ROM directory: " << std::filesystem::absolute(test_path_) << "\n";
            break;
        case CommandType::Invalid:
            std::cout << "Invalid command. Use -help for command list\n";
        }
    }

    void Application::cleanup()
    {

    }
}
