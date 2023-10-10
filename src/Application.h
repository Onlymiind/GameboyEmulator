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

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <X11/Xlib.h>
#include <deque>
#include <filesystem>
#include <queue>
#include <string_view>


namespace emulator {
    constexpr double g_cycles_per_second = 4'000'000;

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

        void drawMenu();

        template<typename Element>
        void pushRecent(std::deque<Element>& cont, const Element& elem) {
            cont.push_back(elem);
            if(cont.size() > recent_cache_size) {
                cont.pop_front();
            }
        }
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
        const std::string extension_ = ".gb";

        GLFWwindow* window_ = nullptr;
    };
}
