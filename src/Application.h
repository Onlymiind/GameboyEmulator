#pragma once
#include "Breakpoint.h"
#include "gb/AddressBus.h"
#include "gb/Emulator.h"
#include "gb/InterruptRegister.h"
#include "gb/Timer.h"
#include "gb/cpu/CPU.h"
#include "gb/cpu/CPUUtils.h"
#include "gb/cpu/Decoder.h"
#include "gb/memory/BasicComponents.h"
#include "utils/Utils.h"

// clang-format off
#include "glad/glad.h"
#include "GLFW/glfw3.h"
// clang-format on

#include <deque>
#include <filesystem>
#include <list>
#include <queue>
#include <string_view>

namespace emulator {
    inline consteval size_t strlen(const char *str) {
        if (!str || str[0] == '\0') {
            return 0;
        } else {
            return 1 + strlen(++str);
        }
    }

    constexpr double g_cycles_per_second = 4'000'000;
    constexpr const char *g_regs_fmt =
        R"(Carry: %d, Half carry: %d
Negative: %d, Zero: %d
A: 0x%.2x, AF: 0x%.4x
C: 0x%.2x, B: 0x%.2x, BC: 0x%.4x
E: 0x%.2x, D: 0x%.2x, DE: 0x%.4x,
H: 0x%.2x, L: 0x%.2x, HL: 0x%.4x,
SP: 0x%.4x, PC: 0x%.4x)";

    constexpr size_t g_instruction_string_buf_size =
        strlen("ffff CALL nz, ffff##111"); // ##111 is needed to accomodate for
                                           // Dear ImGui ids

    constexpr std::string_view g_rom_extension = ".gb";

    constexpr size_t g_recent_cache_size = 10;

    struct InstructionData {
        gb::cpu::RegisterFile registers;
        gb::cpu::Instruction instruction;
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
        void drawMemoryView();

        template <typename Element> void pushRecent(std::list<Element> &cont, const Element &elem) {
            cont.push_back(elem);
            if (cont.size() > g_recent_cache_size) {
                cont.pop_front();
            }
        }

        void addPCBreakpoint(uint16_t address);
        void addMemoryBreakpoint();

        void printInstruction(StringBuffer<g_instruction_string_buf_size> &buf, size_t idx);

      private:
        gb::Emulator emulator_;

        std::vector<std::filesystem::path> roms_;
        std::list<std::filesystem::path> recent_roms_;
        RingBuffer<InstructionData, g_recent_cache_size> recent_instructions_;

        GLFWwindow *window_ = nullptr;

        std::vector<uint16_t> pc_breakpoints_;
        MemoryBreakpoints memory_breakpoints_{[this]() { single_step_ = true; }};

        // buffers for GUI
        MemoryBreakpointData memory_breakpoint_data_;
        std::string new_romdir_;
        std::optional<gb::cpu::RegisterFile> registers_to_print_;

        bool is_running_ = true;
        bool gui_init_ = false;
        bool single_step_ = true;

        int refresh_rate_ = 60;
        uint16_t mem_range_[2] = {0, 0};
    };
} // namespace emulator
