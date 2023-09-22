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
#include "gb/Emulator.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <filesystem>
#include <string_view>


namespace emulator {
    class Application {
    public:
        Application(const Printer& printer, const Reader& reader);
        ~Application();

        void run();

        void draw();

    private:

        void initGUI();
        void update();
        void pollCommands();

        void setROMDirectory(const std::filesystem::path& newPath);
        void listROMs() const;
        void runROM(std::string_view name);
    private:
        gb::Emulator emulator_;
        
        bool is_running_ = true;
        bool emulator_running_ = false;
        bool gui_enabled_ = false;
        bool gui_init_ = false;


        std::filesystem::path ROM_directory_;
        const std::string extension_ = ".gb";
        const Reader& input_reader_;
        const Printer& printer_;

        GLFWwindow* window_ = nullptr;
    };
}
