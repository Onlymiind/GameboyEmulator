#pragma once
#include "gb/RAM.h"
#include "gb/ROM.h"
#include "gb/AddressBus.h"
#include "gb/cpu/CPU.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"

#include "gb/MemoryObject.h"

#include <filesystem>
#include <string_view>


namespace emulator
{
    class Application 
    {
    public:
        Application(const Printer& printer, const Reader& reader);
        ~Application();

        void addMemoryObserver(uint16_t from, uint16_t to, gb::MemoryObject& observer);

        void run();

        void draw();

    private:

        void init();
        void update();
        void pollCommands();
        void cleanup();

        void setROMDirectory(const std::filesystem::path& newPath);
        void listROMs() const;
        void runROM(std::string_view name);

    private:

        gb::RAM RAM_;
        gb::ROM ROM_;
        gb::RAM leftover_;
        gb::AddressBus bus_;
        gb::InterruptRegister interrupt_enable_;
        gb::InterruptRegister interrupt_flags_;
        gb::decoding::Decoder decoder_;
        gb::cpu::SharpSM83 CPU_;
        
        bool is_running_;
        bool emulator_running_;


        std::filesystem::path ROM_directory_;
        const std::string extension_ = ".gb";
        const Reader& input_reader_;
        const Printer& printer_;
    };
}
