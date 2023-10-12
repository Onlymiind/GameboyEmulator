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
#include "utils/Utils.h"

#include <X11/Xlib.h>
#include <deque>
#include <filesystem>
#include <queue>
#include <string_view>
#include <list>


namespace emulator {
    inline consteval size_t strlen(const char* str) {
        if(!str || str[0] == '\0') {
            return 0;
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
C: 0x%.2x, B: 0x%.2x, BC: 0x%.4x
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

    constexpr size_t g_instruction_string_buf_size = strlen("ffff CALL nz, ffff###") + 3; //### and + 3 to accomodate for Dear ImGui ids

    constexpr std::string_view g_rom_extension = ".gb";

    constexpr size_t g_recent_cache_size = 10;

    struct InstructionData {
        gb::cpu::RegisterFile registers;
        gb::cpu::Instruction instruction;
    };

    struct MemoryBreakpointData {
        uint16_t addresses[2] = {0, 0};
        bool read = true;
        bool write = true;
        std::optional<uint8_t> value;
    };

    class Application {
    public:
        Application();
        ~Application();

        void run();

        void draw();

    private:

        void initGUI();
        void update();

        bool setROMDirectory();
        bool runROM(const std::filesystem::path path);

        void drawMainMenu();
        void drawBreakpointMenu();

        template<typename Element>
        void pushRecent(std::deque<Element>& cont, const Element& elem) {
            cont.push_back(elem);
            if(cont.size() > g_recent_cache_size) {
                cont.pop_front();
            }
        }

        void addPCBreakpoint(uint16_t address);
        void addMemoryBreakpoint(uint8_t flags, uint16_t min_address, uint16_t max_address, std::optional<uint8_t> data);
        void resetBreakpoints();

        void printRegisters(gb::cpu::RegisterFile regs);
        void printInstruction(size_t idx, const InstructionData& instr);
    private:

        gb::Emulator emulator_;

        std::vector<std::filesystem::path> roms_;
        std::deque<std::filesystem::path> recent_roms_;
        RingBuffer<InstructionData, g_recent_cache_size> recent_instructions_;

        GLFWwindow* window_ = nullptr;

        //use std::list for pointers to elements must be valid after removal
        std::list<PCBreakpoint> pc_breakpoints_;
        std::list<MemoryBreakpoint> memory_breakpoints_;

        //buffers for GUI
        MemoryBreakpointData memory_breakpoint_data_;
        std::string new_romdir_;

        //string buffers
        std::array<StringBuffer<g_instruction_string_buf_size>, g_recent_cache_size> printed_instructions_; //28 characters should be enough for all instructions
        StringBuffer<g_registers_string_buf_size> printed_regs_;

        bool is_running_ = true;
        bool gui_init_ = false;
        bool single_step_ = true;

        int refresh_rate_ = 60;
    };
}
