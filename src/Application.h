#pragma once
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

#include <filesystem>
#include <queue>
#include <string_view>


namespace emulator {
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

        void pushRecent(const std::filesystem::path& path);
    private:
        gb::Emulator emulator_;
        
        bool is_running_ = true;
        bool gui_init_ = false;


        std::filesystem::path ROM_directory_;
        std::string new_romdir_;
        std::vector<std::filesystem::path> roms_;
        std::deque<std::filesystem::path> recent_roms_;
        const std::string extension_ = ".gb";

        GLFWwindow* window_ = nullptr;
    };
}
