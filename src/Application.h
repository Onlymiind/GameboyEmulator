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

#include <deque>
#include <filesystem>
#include <queue>
#include <string_view>


namespace emulator {
    constexpr size_t g_cycles_per_second = 4'000'000;

    class Application {
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
            if(cont.size() > 10) {
                cont.pop_front();
            }
        }
    private:
        struct InstructionData {
            uint16_t PC = 0;
            gb::cpu::Registers registers;
            gb::cpu::Instruction instruction;
        };

        gb::Emulator emulator_;
        
        bool is_running_ = true;
        bool gui_init_ = false;
        bool single_step_ = true;


        std::filesystem::path ROM_directory_;
        std::string new_romdir_;
        std::vector<std::filesystem::path> roms_;
        std::deque<std::filesystem::path> recent_roms_;
        std::deque<InstructionData> recent_instructions_;
        const std::string extension_ = ".gb";

        GLFWwindow* window_ = nullptr;
    };
}
