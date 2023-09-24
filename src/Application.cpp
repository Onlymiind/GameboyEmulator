#include "Application.h"
#include "utils/Utils.h"
#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "gb//memory/BasicComponents.h"
#include "gb/memory/Memory.h"
#include "gb/Timer.h"
#include "gb/cpu/Decoder.h"

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
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

        ImGui::Begin("name", nullptr, ImGuiWindowFlags_NoResize 
            | ImGuiWindowFlags_NoTitleBar 
            | ImGuiWindowFlags_NoMove 
            | ImGuiWindowFlags_MenuBar
        );
        drawMenu();
        ImGui::End();

        //ImGui::ShowDemoWindow();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    void Application::drawMenu() {
        if(!ImGui::BeginMenuBar()) {
            return;
        }

        if(ImGui::BeginMenu("File")) {

            if(ImGui::BeginMenu("Change ROM Directory")) {

                ImGui::TextWrapped("%s", ROM_directory_.c_str());
                ImGui::InputText("new ROM directory", &new_romdir_);

                if(ImGui::Button("accept")) {
                    if(!setROMDirectory(new_romdir_)) {
                        std::cout << "failed to change ROM directory\n";
                    }
                    new_romdir_.clear();
                }

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Available ROMs")) {
                for(const auto& rom : roms_) {
                    if(ImGui::Selectable(rom.filename().c_str())) {
                        if(!runROM(rom)) {
                            std::cout << "failed to run rom: " << rom << '\n';
                        }
                    }
                }

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Recent ROMs")) {

                for(const auto& rom : recent_roms_) {
                    if(ImGui::Selectable(rom.filename().c_str())) {
                        if(!runROM(rom)) {
                            std::cout << "failed to run rom: " << rom << '\n';
                        }
                    }
                }

                ImGui::EndMenu();
            }

            if(ImGui::Selectable("Stop execution")) {
                emulator_.stop();
            }

            if(ImGui::Selectable("Quit")) {
                is_running_ = false;
            }


            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    Application::Application() {
        initGUI();
    }

    void Application::run() {
        while (is_running_) {

            if (emulator_running_) {
                update();
            }

            draw();

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
            std::cout << "failed to load OpenGL" << std::endl;
            is_running_ = false;
            return;
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
        try {
            emulator_.tick();
        } catch (const std::exception& e) {
            std::cout << "exception occured during emulator update: " << e.what() << std::endl;
        }
    }

    bool Application::setROMDirectory(const std::filesystem::path& newPath) {
        if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath)) {
            ROM_directory_ = newPath;
            roms_.clear();
            for(const auto& item : std::filesystem::directory_iterator(ROM_directory_)) {
                if(item.is_regular_file() && item.path().extension() == extension_) {
                    roms_.push_back(item.path());
                }
            }
            return true;
        }
        
        return false;
    }

    bool Application::runROM(const std::filesystem::path path) {

        if(!std::filesystem::exists(path)) {
            return false;
        }
        
        std::vector<uint8_t> data = readFile(path);
        if(data.empty()) {
            return false;
        }

        pushRecent(path);


        emulator_.reset();
        emulator_.setROM(std::move(data));
        emulator_.start();

        return true;
    }

    void Application::pushRecent(const std::filesystem::path& path) {
        recent_roms_.push_back(path);
        if(recent_roms_.size() > 10) {
            recent_roms_.pop_front();
        }
    }
}
