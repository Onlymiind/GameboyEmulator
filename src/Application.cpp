#include "Application.h"
#include "Breakpoint.h"
#include "gb/Emulator.h"
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
#include <cstdint>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <filesystem>
#include <exception>
#include <cstdio>

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
        drawMainMenu();

        ImGui::Columns(3);
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
        ImGui::NextColumn();
        drawBreakpointMenu();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    void Application::drawMainMenu() {
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

    void Application::drawBreakpointMenu() {
        uint16_t pc_break = 0;
        ImGui::Text("Add PC breakpoint:");
        if(ImGui::InputScalar("###Add PC breakpoint input", ImGuiDataType_U16, &pc_break, 
            nullptr, nullptr, "%.4x", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
                addPCBreakpoint(pc_break);
        }
        {
            ImGui::Text("PC breakpoints: ");
            auto delete_it = pc_breakpoints_.end();
            for(auto it = pc_breakpoints_.begin(); it != pc_breakpoints_.end(); ++it) {
                std::string buf(6, '0');
                sprintf(buf.data(), "0x%.4x", it->getAddress());
                ImGui::Selectable(buf.c_str());
                if(ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                    delete_it = it;
                }
            }
            if(delete_it != pc_breakpoints_.end()) {
                pc_breakpoints_.erase(delete_it);
            }
        }
    }

    Application::Application() 
        : emulator_(false)
    {
        initGUI();
    }

    void Application::run() {
        size_t updates_per_frame = size_t(g_cycles_per_second / size_t(refresh_rate_));
        int frame = 0;
        while (is_running_) {
            double time_start = glfwGetTime();

            if(ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
                single_step_ = !single_step_;
            }

            if(single_step_ && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
                update();
                while(!emulator_.instructionFinished()) {
                    update();
                }
            } else if(!single_step_) {
                for(int i = 0; !emulator_.terminated() && i < updates_per_frame; ++i) {
                    update();
                    if(single_step_) {
                        break;
                    }
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
                for(auto& br : pc_breakpoints_) {
                    if(br.isHit()) {
                        single_step_ = true;
                    }
                }
                for(auto& br: memory_breakpoints_) {
                    if(br.isHit()) {
                        single_step_ = true;
                    }
                }
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
                if(item.is_regular_file() && item.path().extension() == g_rom_extension) {
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

    std::list<PCBreakpoint>::iterator Application::addPCBreakpoint(uint16_t address) {
        pc_breakpoints_.push_front(PCBreakpoint(emulator_, address));
        return pc_breakpoints_.begin();
    }

    std::list<MemoryBreakpoint>::iterator Application::addMemoryBreakpoint(uint8_t flags, uint16_t min_address, uint16_t max_address, std::optional<uint8_t> data) {
        memory_breakpoints_.push_front(MemoryBreakpoint(min_address, max_address, flags, data));
        return memory_breakpoints_.begin();
    }

    void Application::resetBreakpoints() {
        for(auto& br : memory_breakpoints_) {
            br.reset();
        }
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
