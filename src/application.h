#ifndef GB_EMULATOR_SRC_APPLICATION_HDR_
#define GB_EMULATOR_SRC_APPLICATION_HDR_

#include "breakpoint.h"
#include "disassembler.h"
#include "gb/address_bus.h"
#include "gb/cpu/cpu.h"
#include "gb/cpu/cpu_utils.h"
#include "gb/cpu/decoder.h"
#include "gb/emulator.h"
#include "gb/interrupt_register.h"
#include "gb/memory/basic_components.h"
#include "gb/timer.h"
#include "renderer.h"
#include "util/util.h"

// clang-format off
#include "glad/gl.h"
#include "GLFW/glfw3.h"
// clang-format on

#include <cstdint>
#include <deque>
#include <filesystem>
#include <list>
#include <memory>
#include <queue>
#include <string_view>

namespace emulator {

    constexpr size_t g_registers_buffer_size = sizeof("Carry: 1, Half carry: 1\n"
                                                      "Negative: 1, Zero: 1\n"
                                                      "A: ff, AF: ffff\n"
                                                      "C: ff, B: ff, BC: ffff\n"
                                                      "E: ff, D: ff, DE: ffff\n"
                                                      "H: ff, L: ff, HL: ffff\n"
                                                      "SP: ffff, PC: fff\n"
                                                      "IME: 1");

    constexpr size_t g_instruction_string_buf_size = sizeof("ffff CALL nz, ffff##111"); // ##111 is needed to accomodate
                                                                                        // for Dear ImGui ids

    constexpr std::string_view g_rom_extension = ".gb";

    constexpr size_t g_recent_cache_size = 10;

    class Application {
      public:
        Application();
        ~Application();

        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;
        Application(Application &&) = delete;
        Application &operator=(Application &&) = delete;

        void run();

        void draw();

      private:
        void initGUI();
        void update();

        void advanceFrame();

        bool setROMDirectory();
        bool runROM(const std::filesystem::path path);

        void drawMainMenu();
        void drawDebuggerMenu();
        void drawBreakpointMenu();
        void drawMemoryView();
        void drawEmulatorView();
        void drawMemoryRegion(gb::MemoryObjectType region);

        template <typename Element>
        void pushRecent(std::list<Element> &cont, const Element &elem) {
            cont.push_back(elem);
            if (cont.size() > g_recent_cache_size) {
                cont.pop_front();
            }
        }

        void addPCBreakpoint(uint16_t address);
        void addMemoryBreakpoint();

        void printInstruction(StringBuffer &buf, gb::cpu::Instruction instr, std::optional<size_t> idx = {});

      private:
        gb::Emulator emulator_;

        std::vector<std::filesystem::path> roms_;
        std::list<std::filesystem::path> recent_roms_;
        RingBuffer<gb::cpu::Instruction, g_recent_cache_size> recent_instructions_;

        GLFWwindow *window_ = nullptr;

        std::vector<uint16_t> pc_breakpoints_;
        MemoryBreakpoints memory_breakpoints_{[this]() { single_step_ = true; }};
        std::unique_ptr<renderer::Renderer> emulator_renderer_;

        std::pair<uint16_t, uint16_t> current_rom_banks_{0, 1};
        uint16_t current_ram_bank_ = 0;

        Disassembler disassembler_;

        // buffers for GUI
        MemoryBreakpointData memory_breakpoint_data_;
        std::string new_romdir_;
        std::optional<std::pair<gb::cpu::RegisterFile, bool>> registers_to_print_;
        StringBuffer buffer_;
        StringBuffer disasm_buffer_;

        bool is_running_ = true;
        bool gui_init_ = false;
        bool single_step_ = true;
        bool frame_finished_ = false;

        int refresh_rate_ = 60;
    };
} // namespace emulator

#endif
