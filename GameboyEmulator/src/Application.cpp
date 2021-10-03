#include "Application.h"
#include "utils/Utils.h"
#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "gb//memory/BasicComponents.h"
#include "gb/memory/Memory.h"
#include "utils/FileManager.h"
#include "gb/Timer.h"
#include "gb/cpu/Operation.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"

#include <string>
#include <filesystem>
#include <exception>

#define BIND_READ(x, func) std::bind(func, &x, std::placeholders::_1)
#define BIND_WRITE(x, func) std::bind(func, &x, std::placeholders::_1, std::placeholders::_2)

namespace emulator
{
    void Application::draw()
    {

    }

    Application::Application(const Printer& printer, const Reader& reader) :
        RAM_(computeSizeFromAddresses(0x8000, 0xFEFF)), ROM_(), leftover_(computeSizeFromAddresses(0xFF80, 0xFFFD)),
        bus_(), CPU_(bus_, interrupt_enable_, interrupt_flags_, decoder_), is_running_(true), emulator_running_(false),
        input_reader_(reader), printer_(printer)
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
        bus_.connect(gb::MemoryController(0x8000, 0xFF0E, RAM_));
        bus_.connect(gb::MemoryController(0xFF0F, 0xFF0F, interrupt_enable_));
        bus_.connect(gb::MemoryController(0xFF10, 0xFFFE, leftover_));
        bus_.connect(gb::MemoryController(0xFFFF, 0xFFFF, interrupt_flags_));

        printer_.printTitle();
    }

    void Application::addMemoryObserver(uint16_t from, uint16_t to, gb::MemoryObject& observer)
    {
        bus_.addObserver(gb::MemoryController(from, to, observer));
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
        printer_.print('>');

        Command cmd = input_reader_.parse(input_reader_.getLine());

        switch (cmd.type)
        {
            case CommandType::List:
                listROMs();
                break;
            case CommandType::Help:
                printer_.printHelp();
                break;
            case CommandType::SetRomDir:
                setROMDirectory(cmd.argument);
                break;
            case CommandType::Quit:
                is_running_ = false;
                break;
            case CommandType::RunRom:
                runROM(cmd.argument);
                break;
            case CommandType::Config:
                printer_.println("Current ROM directory: ", std::filesystem::absolute(ROM_directory_));
                break;
            case CommandType::Invalid:
                printer_.reportError(InputError::InvalidCommand);
        }
    }

    void Application::setROMDirectory(const std::filesystem::path& newPath)
    {
        if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath))
        {
            ROM_directory_ = newPath;
        }
        else
        {
            printer_.reportError(InputError::InvalidDirectory);
        }
    }

    void Application::listROMs() const
    {
        if(!std::filesystem::exists(ROM_directory_))
        {
            printer_.reportError(InputError::InvalidDirectory);
            return;
        }

        for (const auto& file : std::filesystem::directory_iterator(ROM_directory_))
        {
            if (file.path().extension().string() == ".gb")
            {
                printer_.println("ROM: ", file.path().filename());
            }
        }
    }

    void Application::runROM(std::string_view name)
    {
        std::filesystem::path ROM_path = ROM_directory_;
        ROM_path = ROM_path / std::filesystem::path(name).replace_extension(extension_);
        if (std::filesystem::exists(ROM_path))
        {
            ROM_.setData(FileManager::readFile(ROM_path.string()));
            CPU_.reset();
            emulator_running_ = true;
        }
        else
        {
            printer_.reportError(InputError::InvalidRomName);
            printer_.println("ROM: ", ROM_path);
        }
    }

    void Application::cleanup()
    {

    }
}
