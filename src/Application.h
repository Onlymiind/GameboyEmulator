#pragma once
#include "gb/memory/BasicComponents.h"
#include "gb/AddressBus.h"
#include "gb/cpu/CPU.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"
#include "gb/Timer.h"
#include "gb/memory/Memory.h"

#include "GLFW/glfw3.h"

#include <filesystem>
#include <string_view>


namespace emulator
{
    class Application 
    {
    public:
        Application(const Printer& printer, const Reader& reader, bool exit_on_infinite_loop = false);
        ~Application();

        void addMemoryObserver(uint16_t from, uint16_t to, gb::MemoryObject& observer);

        void run();

        void draw();

    private:

        void init();
        void update();
        void pollCommands();

        void setROMDirectory(const std::filesystem::path& newPath);
        void listROMs() const;
        void runROM(std::string_view name);
    private:
        struct MemoryObjectInfo
        {
            uint16_t min_address, max_address = 0;
            uint16_t size = static_cast<uint16_t>(max_address - min_address + 1);
        };

        struct
        {
            MemoryObjectInfo ROM = {0x0000, 0x7FFF};
            MemoryObjectInfo RAM = {0x8000, 0xFF03};
            MemoryObjectInfo timer = {0xFF04, 0xFF07};
            MemoryObjectInfo leftover2 = {0xFF08, 0xFF0E};
            MemoryObjectInfo interrupt_enable = {0xFF0F, 0xFF0F};
            MemoryObjectInfo leftover = {0xFF10, 0xFFFE};
            MemoryObjectInfo interrupt_flags = {0xFFFF, 0xFFFF};
        } const memory_map_;

        gb::RAM RAM_;
        gb::ROM ROM_;
        gb::RAM leftover_;
        gb::RAM leftover2_;
        gb::AddressBus bus_;
        gb::InterruptRegister interrupt_enable_;
        gb::InterruptRegister interrupt_flags_;
        gb::decoding::Decoder decoder_;
        gb::cpu::SharpSM83 CPU_;
        gb::Timer timer_;
        
        bool is_running_ = false;
        bool emulator_running_ = false;
        bool exit_on_infinite_loop_ = true;
        bool gui_ = false;


        std::filesystem::path ROM_directory_;
        const std::string extension_ = ".gb";
        const Reader& input_reader_;
        const Printer& printer_;

        GLFWwindow* window_ = nullptr;
    };
}
