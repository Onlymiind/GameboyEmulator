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
        for(size_t i = 0; i < recent_instructions_.size(); ++i) {
            printInstruction(i, recent_instructions_[i]);
            if(ImGui::Selectable(printed_instructions_[i].data())) {
                printRegisters(recent_instructions_[i].registers);
            }
        }
        if(single_step_) {
            ImGui::TextUnformatted("Single stepping");
        }

        ImGui::NextColumn();
        ImGui::TextUnformatted(printed_regs_.data());
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
                
                if(ImGui::InputText("###newdir", &new_romdir_, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    if(!setROMDirectory()) {
                        std::cout << "failed to change ROM directory\n";
                    }
                    new_romdir_.clear();
                }

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Available ROMs")) {
                if(!roms_.empty()) {
                    ImGui::TextWrapped("%s", roms_[0].parent_path().c_str());
                }
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
                printed_regs_[0] = '\0';
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
        ImGui::TextUnformatted("Add PC breakpoint:");
        if(ImGui::InputScalar("###Add PC breakpoint input", ImGuiDataType_U16, &pc_break, 
            nullptr, nullptr, "%.4x", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
                addPCBreakpoint(pc_break);
        }
        {
            ImGui::Text("PC breakpoints: ");
            auto delete_it = pc_breakpoints_.end();
            std::string buf(6, '0');
            for(auto it = pc_breakpoints_.begin(); it != pc_breakpoints_.end(); ++it) {
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

        ImGui::TextUnformatted("Add memory breakpoint:");
        ImGui::InputScalarN("###addresses", ImGuiDataType_U16, memory_breakpoint_data_.addresses, 2,
            nullptr, nullptr, "%.4x", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue);
        if(memory_breakpoint_data_.addresses[1] < memory_breakpoint_data_.addresses[0]) {
            memory_breakpoint_data_.addresses[1] = memory_breakpoint_data_.addresses[0];
        }
        ImGui::Checkbox("Read", &memory_breakpoint_data_.read);
        ImGui::Checkbox("Write", &memory_breakpoint_data_.write);

        uint8_t temp_value = 0;
        if(ImGui::InputScalar("value", ImGuiDataType_U8, &temp_value, 
            nullptr, nullptr, "%.2x", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            memory_breakpoint_data_.value = temp_value;
        }

        if(ImGui::Button("Add")) {
            uint8_t flags = 0;
            if(memory_breakpoint_data_.read) {
                flags |= uint8_t(MemoryBreakpointFlags::READ);
            }
            if(memory_breakpoint_data_.write) {
                flags |= uint8_t(MemoryBreakpointFlags::WRITE);
            }
            addMemoryBreakpoint(flags, memory_breakpoint_data_.addresses[0], memory_breakpoint_data_.addresses[1], memory_breakpoint_data_.value);
            memory_breakpoint_data_ = MemoryBreakpointData{};
        }
        {
            ImGui::Text("Memory breakpoints: ");
            auto delete_it = memory_breakpoints_.end();
            std::string buf(strlen("Min address: 0xffff\nMax address: 0xffff\nRead: 1, write: 1, value: 0xff"), '\0');
            for(auto it = memory_breakpoints_.begin(); it != memory_breakpoints_.end(); ++it) {
                int written = sprintf(buf.data(), "Min address: 0x%.4x\nMax address: 0x%.4x\nRead: %d, write: %d",
                    it->getMaxAddress(), it->getMinAddress(), 
                    (it->getFlags() & uint8_t(MemoryBreakpointFlags::READ)) != 0,
                    (it->getFlags() & uint8_t(MemoryBreakpointFlags::WRITE)) != 0);
                if(it->getValue()) {
                    sprintf(buf.data() + written, ", value: 0x%.2x", *it->getValue());
                }
                ImGui::Selectable(buf.c_str());
                if(ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                    delete_it = it;
                }
            }
            if(delete_it != memory_breakpoints_.end()) {
                memory_breakpoints_.erase(delete_it);
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

            if(single_step_ && ImGui::IsKeyPressed(ImGuiKey_S)) {
                update();
                while(!emulator_.instructionFinished()) {
                    update();
                }
            } else if(!single_step_) {
                for(int i = 0; !emulator_.terminated() && i < updates_per_frame; ++i) {
                    update();
                    if(single_step_) {
                        //run current instruction until completion
                        while(!emulator_.instructionFinished()) {
                            update();
                        }
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
                        br.reset();
                    }
                }
            }
            emulator_.tick();
            if(fetch_data) {
                instr_data.instruction = emulator_.getLastInstruction();
                recent_instructions_.push_back(instr_data);
            }
        } catch (const std::exception& e) {
            std::cout << "exception occured during emulator update: " << e.what() << std::endl;
        }
    }

    bool Application::setROMDirectory() {
        std::filesystem::path path(new_romdir_);
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            roms_.clear();
            for(const auto& item : std::filesystem::directory_iterator(path)) {
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

    void Application::addPCBreakpoint(uint16_t address) {
        pc_breakpoints_.push_front(PCBreakpoint(emulator_, address));
    }

    void Application::addMemoryBreakpoint(uint8_t flags, uint16_t min_address, uint16_t max_address, std::optional<uint8_t> data) {
        memory_breakpoints_.push_front(MemoryBreakpoint(min_address, max_address, flags, data));
        emulator_.addMemoryObserver(gb::MemoryController{min_address, max_address, *memory_breakpoints_.begin()});
    }

    void Application::resetBreakpoints() {
        for(auto& br : memory_breakpoints_) {
            br.reset();
        }
    }

    void Application::printInstruction(size_t idx, const InstructionData& instr_data) {
        using namespace gb::decoding;
        auto& instr = instr_data.instruction;
        auto& buf = printed_instructions_[idx];
        std::stringstream out;
        out.rdbuf()->pubsetbuf(buf.data(), buf.capacityWithNullChar());

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
                if(instr.src.get<Registers>() == Registers::HL) {
                    out << "--";
                }
                break;
            case LoadSubtype::LD_INC:
                if(instr.src.get<Registers>() == Registers::HL) {
                    out << "++";
                }
                break;
            }
        }

        out << "###" << idx; //for Dear ImGui ids

    }

    void Application::printRegisters(gb::cpu::RegisterFile regs) {
        using enum gb::cpu::Flags;
        sprintf(printed_regs_.data(), g_regs_fmt, 
            regs.getFlag(CARRY), regs.getFlag(HALF_CARRY),
            regs.getFlag(NEGATIVE), regs.getFlag(ZERO),
            regs.A(), regs.AF(), regs.C(), regs.B(), regs.BC(),
            regs.E(), regs.D(), regs.DE(),
            regs.H(), regs.L(), regs.HL(),
            regs.SP, regs.PC
        );
    }
}
