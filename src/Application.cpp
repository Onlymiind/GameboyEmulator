#include "Application.h"
#include "utils/Utils.h"
#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "gb//memory/BasicComponents.h"
#include "gb/memory/Memory.h"
#include "gb/Timer.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


#include <string>
#include <filesystem>
#include <exception>

namespace emulator {
    void Application::draw() {
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("name", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
        ImGui::End();        


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    Application::Application(const Printer& printer, const Reader& reader)
        :input_reader_(reader), printer_(printer) 
    {
        printer_.printTitle();
    }

    void Application::run() {
        while (is_running_) {

            if (emulator_running_) {
                update();
            }
            else if(gui_enabled_) {
                draw();
            }
            else {
                pollCommands();
            }

            if(window_ && glfwWindowShouldClose(window_)) {
                is_running_ = false;
            }
        }
    }

    void Application::initGUI() {
        if(gui_init_) {
            return;
        }
        
        glfwInit();
        window_ = glfwCreateWindow(600, 600, "emulator", nullptr, nullptr);
        glfwMakeContextCurrent(window_);
        if(gladLoadGL() == 0) {
            printer_.reportError("failed to load OpenGL", 0);
        }


        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init();
        gui_init_ = true;
    }

    Application::~Application() {
        if(gui_init_) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            glfwDestroyWindow(window_);
            glfwTerminate();
        }
    }

    void Application::update() {
        emulator_.tick();
        if(emulator_.terminated()) {
            const auto& err = emulator_.getErrorDescription();
            if(!err.empty()) {
                printer_.reportError(err, 0);
            }
        }
    }

    void Application::pollCommands() {
        printer_.print('>');

        Command cmd = input_reader_.parse(input_reader_.getLine());

        switch (cmd.type) {
            case CommandType::List:
                listROMs();
                break;
            case CommandType::Help:
                printer_.printHelp();
                break;
            case CommandType::SetRomDir:
                setROMDirectory(cmd.argument);
                break;
            case CommandType::Quit:
                is_running_ = false;
                break;
            case CommandType::RunRom:
                runROM(cmd.argument);
                break;
            case CommandType::Config:
                printer_.println("Current ROM directory: ", std::filesystem::absolute(ROM_directory_));
                break;
            case CommandType::Invalid:
                printer_.reportError(InputError::InvalidCommand);
                break;
            case CommandType::LaunchGUI:
                initGUI();
                gui_enabled_ = true;
                break;
        }
    }

    void Application::setROMDirectory(const std::filesystem::path& newPath) {
        if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath)) {
            ROM_directory_ = newPath;
        }
        else {
            printer_.reportError(InputError::InvalidDirectory);
        }
    }

    void Application::listROMs() const {
        if(!std::filesystem::exists(ROM_directory_)) {
            printer_.reportError(InputError::InvalidDirectory);
            return;
        }

        for (const auto& file : std::filesystem::directory_iterator(ROM_directory_)) {
            if (file.path().extension().string() == ".gb") {
                printer_.println("ROM: ", file.path().filename());
            }
        }
    }

    void Application::runROM(std::string_view name) {
        std::filesystem::path ROM_path;
        if(!ROM_directory_.empty()) {
            ROM_path = ROM_directory_;
        }
        ROM_path = ROM_path / std::filesystem::path(name).replace_extension(extension_);
        if (std::filesystem::exists(ROM_path)) {
            emulator_.reset();
            emulator_.setROM(readFile(ROM_path));
            emulator_.start();
        }
        else {
            printer_.reportError(InputError::InvalidRomName);
            printer_.println("ROM: ", ROM_path);
        }
    }
}
