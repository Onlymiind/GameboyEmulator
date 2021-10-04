#include "Application.h"
#include "utils/Utils.h"
#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "gb//memory/BasicComponents.h"
#include "gb/memory/Memory.h"
#include "gb/Timer.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"

#include <string>
#include <filesystem>
#include <exception>

namespace emulator
{
    void Application::draw()
    {
        //TODO
    }

    Application::Application(const Printer& printer, const Reader& reader) :
        RAM_(memory_map_.RAM.size), ROM_(), leftover_(memory_map_.leftover.size), leftover2_(memory_map_.leftover2.size),
        bus_(), CPU_(bus_, interrupt_enable_, interrupt_flags_, decoder_), timer_(interrupt_flags_),
        is_running_(true), emulator_running_(false),
        input_reader_(reader), printer_(printer)
    {
        init();
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
        bus_.connect({memory_map_.ROM.min_address, memory_map_.ROM.max_address, ROM_});
        bus_.connect({memory_map_.RAM.min_address, memory_map_.RAM.max_address, RAM_});
        bus_.connect({memory_map_.timer.min_address, memory_map_.timer.max_address, timer_});
        bus_.connect({memory_map_.leftover2.min_address, memory_map_.leftover2.max_address, leftover2_});
        bus_.connect({memory_map_.interrupt_enable.min_address, memory_map_.interrupt_enable.max_address, interrupt_enable_});
        bus_.connect({memory_map_.leftover.min_address, memory_map_.leftover.max_address, leftover_});
        bus_.connect({memory_map_.interrupt_flags.min_address, memory_map_.interrupt_flags.max_address, interrupt_flags_});

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

        try
        {
            CPU_.tick();
            for(int i = 0; i < 4; ++i)
            {
                timer_.update();
            }
        }
        catch(const std::exception& e)
        {
            emulator_running_ = false;
            printer_.reportError(e.what(), CPU_.getProgramCounter());
            return;
        }

        if(CPU_.isFinished())
        {
            if(oldPC == CPU_.getProgramCounter())
            {
                emulator_running_ = false;
                printer_.reportError("Reached infinite loop", CPU_.getProgramCounter());
            }

            oldPC = CPU_.getProgramCounter();
        }
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
            ROM_.setData(readFile(ROM_path));
            CPU_.reset();
            emulator_running_ = true;
        }
        else
        {
            printer_.reportError(InputError::InvalidRomName);
            printer_.println("ROM: ", ROM_path);
        }
    }
}
