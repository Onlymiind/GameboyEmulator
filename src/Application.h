#pragma once
#include "gb/cpu/CPUUtils.h"
#include "gb/memory/BasicComponents.h"
#include "gb/AddressBus.h"
#include "gb/cpu/CPU.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Decoder.h"
#include "gb/Timer.h"
#include "gb/memory/Memory.h"
#include "gb/Emulator.h"
#include "Breakpoint.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <X11/Xlib.h>
#include <deque>
#include <filesystem>
#include <queue>
#include <string_view>
#include <list>


namespace emulator {
    inline consteval size_t strlen(const char* str) {
        if(!*str) {
            return 1;
        } else {
            return 1 + strlen(++str);
        }
    }

    constexpr double g_cycles_per_second = 4'000'000;
    constexpr const char* g_regs_fmt = 
R"(Flags:
Carry: %d, Half carry: %d
Negative: %d, Zero: %d
A: 0x%.2x, AF: 0x%.4x
C: 0x%.2x, B: 0x%.2x, BC: 0x%.4xdeque
deque
E: 0x%.2x, D: 0x%.2x, DE: 0x%.4x,
H: 0x%.2x, L: 0x%.2x, HL: 0x%.4x,
SP: 0x%.4x, PC: 0x%.4x)";

    constexpr size_t g_registers_string_buf_size = strlen(
R"(Flags:
Carry: 1, Half carry: 1
Negative: 1, Zero: 1
A: 0xff, AF: 0xffff
C: 0xff, B: 0xff, BC: 0xffff
E: 0xff, D: 0xff, DE: 0xffff,
H: 0xff, L: 0xff, HL: 0xffff,
SP: 0xffff, PC: 0xffff)"
    );

    constexpr std::string_view g_rom_extension = ".gb";

    struct InstructionData {
        gb::cpu::RegisterFile registers;
        gb::cpu::Instruction instruction;
    };

    void printInstruction(std::ostream& out, const InstructionData& instr);
    void printRegisters(std::ostream& out, gb::cpu::RegisterFile regs);

    class Application {
        static constexpr size_t recent_cache_size = 10;
    public:
        Application();
        ~Application();

        void run();

        void draw();

    private:

        void initGUI();
        void update();

        bool setROMDirectory(const std::filesystem::path& newPath);
        bool runROM(const std::filesystem::path path);

        void drawMainMenu();
        void drawBreakpointMenu();

        template<typename Element>
        void pushRecent(std::deque<Element>& cont, const Element& elem) {
            cont.push_back(elem);
            if(cont.size() > recent_cache_size) {
                cont.pop_front();
            }
        }

        std::list<PCBreakpoint>::iterator addPCBreakpoint(uint16_t address);
        std::list<MemoryBreakpoint>::iterator addMemoryBreakpoint(uint8_t flags, uint16_t min_address, uint16_t max_address, std::optional<uint8_t> data);
        void resetBreakpoints();
    private:

        gb::Emulator emulator_;

        bool is_running_ = true;
        bool gui_init_ = false;
        bool single_step_ = true;

        int refresh_rate_ = 60;


        std::filesystem::path ROM_directory_;
        std::string new_romdir_;
        std::vector<std::filesystem::path> roms_;
        std::deque<std::filesystem::path> recent_roms_;
        std::deque<InstructionData> recent_instructions_;
        std::array<std::string, recent_cache_size> printed_instructions_;
        std::string printed_regs_;

        GLFWwindow* window_ = nullptr;

        //use std::list for pointers to elements must be valid after removal
        std::list<PCBreakpoint> pc_breakpoints_;
        std::list<MemoryBreakpoint> memory_breakpoints_;
    };
}
