#include "Application.h"
#include "gb/cpu/CPUUtils.h"
#include "gb/cpu/Operation.h"
#include "utils/Utils.h"
#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "gb/memory/BasicComponents.h"
#include "gb/memory/Memory.h"
#include "gb/Timer.h"
#include "gb/cpu/Decoder.h"

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


#include <cmath>
#include <iomanip>
#include <ios>
#include <sstream>
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

        ImGui::Columns(2);
        std::stringstream instr_out;
        for(size_t i = 0; i < recent_instructions_.size(); ++i) {
            printInstruction(instr_out, recent_instructions_[i]);
            instr_out << "###" << i; //For Dear ImGui ids
            printed_instructions_[i] = std::move(instr_out).str();
            instr_out.clear();
            if(ImGui::Selectable(printed_instructions_[i].c_str())) {
                printRegisters(instr_out, recent_instructions_[i].registers);
                printed_regs_ = std::move(instr_out).str();
                instr_out.clear();
            }
        }

        ImGui::NextColumn();
        if(!printed_regs_.empty()) {
            ImGui::TextUnformatted(printed_regs_.c_str());
        }

        ImGui::End();

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
                if(!ROM_directory_.empty()) {
                    ImGui::TextWrapped("%s", ROM_directory_.c_str());
                }
                if(ImGui::InputText("###newdir", &new_romdir_, ImGuiInputTextFlags_EnterReturnsTrue)) {
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

            if(ImGui::Selectable("Reset")) {
                recent_instructions_.clear();
                printed_regs_.clear();
                emulator_.reset();
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
        size_t updates_per_frame = size_t(g_cycles_per_second / size_t(refresh_rate_));
        int frame = 0;
        while (is_running_) {
            double time_start = glfwGetTime();

            if(single_step_ && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
                update();
                while(!emulator_.instructionFinished()) {
                    update();
                }
            } else if(!single_step_) {
                for(int i = 0; !emulator_.terminated() && i < updates_per_frame; ++i) {
                    update();
                }

                ++frame;
                if(frame == refresh_rate_) {
                    frame = 0;
                    size_t leftover_updates = g_cycles_per_second - updates_per_frame * size_t(refresh_rate_);
                    for(int i = 0; !emulator_.terminated() && i < leftover_updates; ++i) {
                        update();
                    }
                }
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
        //TODO: no support for multiple monitors
        refresh_rate_ = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
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
            InstructionData instr_data;
            bool fetch_data = emulator_.instructionFinished();
            if(fetch_data) {
                instr_data.registers = emulator_.getRegisters();
            }
            emulator_.tick();
            if(fetch_data) {
                instr_data.instruction = emulator_.getLastInstruction();
                pushRecent(recent_instructions_, instr_data);
            }
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

        pushRecent(recent_roms_, path);


        emulator_.reset();
        emulator_.setROM(std::move(data));
        emulator_.start();

        return true;
    }

    void printInstruction(std::ostream& out, const InstructionData& instr_data) {
        using namespace gb::decoding;
        auto& instr = instr_data.instruction;

        out << std::hex << instr.pc << ' ' << to_string(instr.type);
        if(instr.condition) {
            out << ' ' << to_string(*instr.condition);
        }

        auto print_arg = [&out](gb::cpu::Instruction::Argument arg) {
            if(arg.empty()) {
                return;
            } else if(arg.is<Registers>()) {
                out << ' ' << to_string(arg.get<Registers>());
            } else if(arg.is<int8_t>()) {
                out << std::setw(0) << std::dec << ' ' << (arg.get<int8_t>() > 0 ? "+" : "") << +arg.get<int8_t>();
            } else if(arg.is<uint8_t>()) {
                out << ' ' << std::setw(2) << std::setfill('0') << std::hex << int(arg.get<uint8_t>());
            } else if(arg.is<uint16_t>()) {
                out << ' ' << std::setw(4) << std::setfill('0') << std::hex << int(arg.get<uint16_t>());
            }
        };


        if(instr.load_subtype) {
            if(*instr.load_subtype == LoadSubtype::LD_Offset_SP) {
                out << " HL,";
            } else if(*instr.load_subtype == LoadSubtype::LD_IO) {
                out << "H";
            }
        }

        print_arg(instr.dst);

        if(instr.load_subtype) {
            switch (*instr.load_subtype) {
            case LoadSubtype::LD_DEC:
                if(instr.dst.get<Registers>() == Registers::HL) {
                    out << "--";
                }
                break;
            case LoadSubtype::LD_INC:
                if(instr.dst.get<Registers>() == Registers::HL) {
                    out << "++";
                }
                break;
            }
        }

        if(!instr.dst.empty()) {
            out << ',';
        }
        print_arg(instr.src);

        if(instr.load_subtype) {
            switch (*instr.load_subtype) {
            case LoadSubtype::LD_DEC:
                if(instr.dst.get<Registers>() == Registers::HL) {
                    out << "--";
                }
                break;
            case LoadSubtype::LD_INC:
                if(instr.dst.get<Registers>() == Registers::HL) {
                    out << "++";
                }
                break;
            }
        }

    }

    void printRegisters(std::ostream& out, gb::cpu::RegisterFile regs) {
        using namespace gb::cpu;
        auto print8 = [&out](uint8_t data) { out << std::setw(2) << std::setfill('0') << std::hex << int(data); };
        auto print16 = [&out](uint16_t data) { out << std::setw(4) << std::setfill('0') << std::hex << data; };
        auto print_reg_pair = [&out, &print8, &print16] (char low, uint8_t low_data, char high, uint8_t high_data, uint16_t double_data) {
            out << low << ": ";
            print8(low_data);
            out << ',' << high << ": ";
            print8(high_data);
            out << ',' << high << low << ": ";
            print16(double_data);
        };

        out << "Flags: \n" << "Carry: " << regs.getFlag(Flags::CARRY) <<
            ", Half carry: " << regs.getFlag(Flags::HALF_CARRY) << '\n' <<
            "Negative: " << regs.getFlag(Flags::NEGATIVE) << 
            ", Zero: " << regs.getFlag(Flags::ZERO) << '\n';
        
        out << "A: ";
        print8(regs.A());
        out << ", AF: ";
        print16(regs.AF());
        out << '\n';
        print_reg_pair('C', regs.C(), 'B', regs.B(), regs.BC());
        out << '\n';
        print_reg_pair('E', regs.E(), 'D', regs.D(), regs.DE());
        out << '\n';
        print_reg_pair('L', regs.L(), 'H', regs.H(), regs.HL());
        out << "\nSP: ";
        print16(regs.SP);
        out << ", PC: ";
        print16(regs.PC);
    }
}
