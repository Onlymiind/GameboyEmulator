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
        bus_(), CPU_(bus_, interrupt_enable_, interrupt_flags_, decoder_), is_running_(true), emulator_running_(false), just_started_(false),
        printer_(std::cout)
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

        printer_.printTitle();
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
                printer_.reportError(e.what(), CPU_.getProgramCounter());
                break;
            }
            
        } while (!CPU_.isFinished());

        if(oldPC == CPU_.getProgramCounter() && emulator_running_)
        {
            emulator_running_ = false;
            printer_.reportError("Reached infinite loop", CPU_.getProgramCounter());
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
                printer_.reportError(InputError::InvalidDirectory);
            }

            break;
        case CommandType::Help:
            printer_.printHelp();
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
                printer_.reportError(InputError::InvalidRomName);
            }

            break;
        case CommandType::Config:
            printer_.print("Current ROM directory: ", std::filesystem::absolute(test_path_));
            break;
        case CommandType::Invalid:
            printer_.reportError(InputError::InvalidCommand);
        }
    }

    void Application::cleanup()
    {

    }
}
